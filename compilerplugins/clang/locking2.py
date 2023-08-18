#!/usr/bin/python3

import re
import io

definitionToTypeMap = dict()
writeFromOutsideConstructorSet = set()
lockedMap = dict()

# clang does not always use exactly the same numbers in the type-parameter vars it generates
# so I need to substitute them to ensure we can match correctly.
normalizeTypeParamsRegex = re.compile(r"type-parameter-\d+-\d+")
def normalizeTypeParams( line ):
    return normalizeTypeParamsRegex.sub("type-parameter-?-?", line)

def parseFieldInfo( tokens ):
    if len(tokens) == 3:
        return (normalizeTypeParams(tokens[1]), tokens[2])
    else:
        return (normalizeTypeParams(tokens[1]), "")

with io.open("workdir/loplugin.locking2.log", "r", buffering=1024*1024) as txt:
    for line in txt:
        tokens = line.strip().split("\t")
        if tokens[0] == "definition:":
            fieldInfo = (normalizeTypeParams(tokens[1]), tokens[2])
            srcLoc = tokens[4]
            # ignore external source code
            if (srcLoc.startswith("external/")):
                continue
            # ignore build folder
            if (srcLoc.startswith("workdir/")):
                continue
            definitionToTypeMap[fieldInfo] = tokens[3]
        elif tokens[0] == "write-outside-constructor:":
            writeFromOutsideConstructorSet.add(parseFieldInfo(tokens))
        elif tokens[0] == "locked:":
            fieldInfo = (normalizeTypeParams(tokens[1]), tokens[2])
            lockedMap[fieldInfo] = tokens[3]
        else:
            print( "unknown line: " + line)

removeLockingSet = set()
for field,srcLoc in lockedMap.items():
    if field in writeFromOutsideConstructorSet:
        continue
    if field not in definitionToTypeMap:
        continue
    fieldType = definitionToTypeMap[field]
    if "std::unique_ptr" in fieldType:
        continue
    if "std::shared_ptr" in fieldType:
        continue
    if "Reference<" in fieldType:
        continue
    if "VclPtr<" in fieldType:
        continue
    removeLockingSet.add((field[0] + " " + field[1] + " " + fieldType, srcLoc))


# sort the results using a "natural order" so sequences like [item1,item2,item10] sort nicely
def natural_sort_key(s, _nsre=re.compile('([0-9]+)')):
    return [int(text) if text.isdigit() else text.lower()
            for text in re.split(_nsre, s)]
# sort by both the source-line and the datatype, so the output file ordering is stable
# when we have multiple items on the same source line
def v_sort_key(v):
    return natural_sort_key(v[1]) + [v[0]]

# sort results by name and line number
tmp6list = sorted(removeLockingSet, key=lambda v: v_sort_key(v))

# print out the results
with open("compilerplugins/clang/locking2.results", "wt") as f:
    for t in tmp6list:
        f.write( t[1] + "\n" )
        f.write( "    " + t[0] + "\n" )


