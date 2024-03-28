#!/usr/bin/python

import re
import io

definitionSet = set()
definitionToSourceLocationMap = dict()
definitionToTypeMap = dict()
writeFromOutsideConstructorSet = set()

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

with io.open("workdir/loplugin.constfields.log", "rb", buffering=1024*1024) as txt:
    for line in txt:
        tokens = line.strip().split("\t")
        if tokens[0] == "definition:":
            access = tokens[1]
            fieldInfo = (normalizeTypeParams(tokens[2]), tokens[3])
            srcLoc = tokens[5]
            # ignore external source code
            if (srcLoc.startswith("external/")):
                continue
            # ignore build folder
            if (srcLoc.startswith("workdir/")):
                continue
            definitionSet.add(fieldInfo)
            definitionToTypeMap[fieldInfo] = tokens[4]
            definitionToSourceLocationMap[fieldInfo] = tokens[5]
        elif tokens[0] == "write-outside-constructor:":
            writeFromOutsideConstructorSet.add(parseFieldInfo(tokens))
        else:
            print( "unknown line: " + line)

# Calculate can-be-const-field set
canBeConstFieldSet = set()
for d in definitionSet:
    if d in writeFromOutsideConstructorSet:
        continue
    srcLoc = definitionToSourceLocationMap[d]
    fieldType = definitionToTypeMap[d]
    if fieldType.startswith("const "):
        continue
    if "std::unique_ptr" in fieldType:
        continue
    if "std::shared_ptr" in fieldType:
        continue
    if "Reference<" in fieldType:
        continue
    if "VclPtr<" in fieldType:
        continue
    if "osl::Mutex" in fieldType:
        continue
    if "::sfx2::sidebar::ControllerItem" in fieldType:
        continue
    canBeConstFieldSet.add((d[0] + " " + d[1] + " " + fieldType, srcLoc))


# sort the results using a "natural order" so sequences like [item1,item2,item10] sort nicely
def natural_sort_key(s, _nsre=re.compile('([0-9]+)')):
    return [int(text) if text.isdigit() else text.lower()
            for text in re.split(_nsre, s)]
# sort by both the source-line and the datatype, so the output file ordering is stable
# when we have multiple items on the same source line
def v_sort_key(v):
    return natural_sort_key(v[1]) + [v[0]]

# sort results by name and line number
tmp6list = sorted(canBeConstFieldSet, key=lambda v: v_sort_key(v))

# print out the results
with open("compilerplugins/clang/constfields.results", "wt") as f:
    for t in tmp6list:
        f.write( t[1] + "\n" )
        f.write( "    " + t[0] + "\n" )


