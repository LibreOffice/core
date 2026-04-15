#!/usr/bin/python3

import re
import io

definitionSet = set()
definitionToSourceLocationMap = dict()
definitionToTypeMap = dict()
castMap = dict()

# clang does not always use exactly the same numbers in the type-parameter vars it generates
# so I need to substitute them to ensure we can match correctly.
normalizeTypeParamsRegex = re.compile(r"type-parameter-\d+-\d+")
def normalizeTypeParams( line ):
    return normalizeTypeParamsRegex.sub("type-parameter-?-?", line)


with io.open("workdir/loplugin.fieldcast.log", "r", buffering=1024*1024) as txt:
    for line in txt:
        tokens = line.strip().split("\t")
        if tokens[0] == "cast:":
            fieldInfo = (normalizeTypeParams(tokens[1]), tokens[2])
            fieldType = tokens[3]
            srcLoc = tokens[4]
            castToType = tokens[5]
            # ignore external source code
            if srcLoc.startswith("external/"):
                continue
            # ignore build folder
            if srcLoc.startswith("workdir/"):
                continue
            definitionSet.add(fieldInfo)
            definitionToTypeMap[fieldInfo] = fieldType
            definitionToSourceLocationMap[fieldInfo] = srcLoc

            if fieldInfo not in castMap:
                castMap[fieldInfo] = castToType
            elif castMap[fieldInfo] != "": # if we are not ignoring it
                # if it is cast to more than one type, mark it as being ignored
                if castMap[fieldInfo] != castToType:
                    castMap[fieldInfo] = ""
        else:
            print( "unknown line: " + line)

outputSet = set()
for k, v in castMap.items():
    if v == "":
        continue
    srcLoc = definitionToSourceLocationMap[k]
    outputSet.add((k[0] + " " + k[1] + " " + definitionToTypeMap[k], srcLoc, v))

# sort the results using a "natural order" so sequences like [item1,item2,item10] sort nicely
def natural_sort_key(s, _nsre=re.compile('([0-9]+)')):
    return [int(text) if text.isdigit() else text.lower()
            for text in re.split(_nsre, s)]
# sort by both the source-line and the datatype, so the output file ordering is stable
# when we have multiple fields declared on the same source line
def v_sort_key(v):
    return natural_sort_key(v[1]) + [v[0]]

# sort results by name and line number
tmp1list = sorted(outputSet, key=lambda v: v_sort_key(v))

# print out the results
with open("compilerplugins/clang/fieldcast.results", "wt") as f:
    for t in tmp1list:
        f.write( t[1] + "\n" )
        f.write( "    " + t[0] + "\n" )
        f.write( "    " + t[2] + "\n" )


