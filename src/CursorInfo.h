#ifndef CursorInfo_h
#define CursorInfo_h

#include "ByteArray.h"
#include "Location.h"
#include "Path.h"
#include "Log.h"
#include "List.h"
#include <clang-c/Index.h>

class CursorInfo;
typedef Map<Location, CursorInfo> SymbolMap;
class CursorInfo
{
public:
    CursorInfo()
        : symbolLength(0), kind(CXCursor_FirstInvalid), isDefinition(false)
    {}
    void clear()
    {
        symbolLength = 0;
        kind = CXCursor_FirstInvalid;
        isDefinition = false;
        targets.clear();
        references.clear();
        symbolName.clear();
    }

    bool dirty(const Set<uint32_t> &dirty)
    {
        bool changed = false;
        Set<Location> *locations[] = { &targets, &references };
        for (int i=0; i<2; ++i) {
            Set<Location> &l = *locations[i];
            Set<Location>::iterator it = l.begin();
            while (it != l.end()) {
                if (dirty.contains(it->fileId())) {
                    changed = true;
                    l.erase(it++);
                } else {
                    ++it;
                }
            }
        }
        return changed;
    }

    bool isValid() const
    {
        return !isEmpty();
    }

    bool isNull() const
    {
        return isEmpty();
    }

    CursorInfo bestTarget(const SymbolMap &map, Location *loc = 0) const;
    Map<Location, CursorInfo> targetInfos(const SymbolMap &map) const;
    Map<Location, CursorInfo> referenceInfos(const SymbolMap &map) const;

    bool isEmpty() const
    {
        assert((symbolLength || symbolName.isEmpty()) && (symbolLength || kind == CXCursor_FirstInvalid)); // these should be coupled
        return !symbolLength && targets.isEmpty() && references.isEmpty();
    }

    bool unite(const CursorInfo &other)
    {
        bool changed = false;
        if (targets.isEmpty() && !other.targets.isEmpty()) {
            targets = other.targets;
            changed = true;
        } else if (!other.targets.isEmpty()) {
            int count = 0;
            targets.unite(other.targets, &count);
            if (count)
                changed = true;
        }

        if (!symbolLength && other.symbolLength) {
            symbolLength = other.symbolLength;
            kind = other.kind;
            isDefinition = other.isDefinition;
            symbolName = other.symbolName;
            changed = true;
        }
        const int oldSize = references.size();
        if (!oldSize) {
            references = other.references;
            if (!references.isEmpty())
                changed = true;
        } else {
            int inserted = 0;
            references.unite(other.references, &inserted);
            if (inserted)
                changed = true;
        }

        return changed;
    }

    ByteArray toString() const;

    unsigned char symbolLength; // this is just the symbol name length e.g. foo => 3
    ByteArray symbolName; // this is fully qualified Foobar::Barfoo::foo
    CXCursorKind kind;
    bool isDefinition;
    Set<Location> targets;
    Set<Location> references;
};

inline Log operator<<(Log log, const CursorInfo &info)
{
    log << info.toString();
    return log;
}

#endif
