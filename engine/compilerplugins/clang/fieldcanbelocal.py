#!/usr/bin/python3

import re
import io

definitionSet = set()
definitionToSourceLocationMap = dict()
definitionToTypeMap = dict()
touchedMap = dict()
excludeSet = set()
sourceLocationSet = set()

# clang does not always use exactly the same numbers in the type-parameter vars it generates
# so I need to substitute them to ensure we can match correctly.
normalizeTypeParamsRegex = re.compile(r"type-parameter-\d+-\d+")
def normalizeTypeParams( line ):
    return normalizeTypeParamsRegex.sub("type-parameter-?-?", line)


with io.open("workdir/loplugin.fieldcanbelocal.log", "r", buffering=1024*1024) as txt:
    for line in txt:
        tokens = line.strip().split("\t")
        if tokens[0] == "definition:":
            fieldInfo = (normalizeTypeParams(tokens[1]), tokens[2])
            fieldType = tokens[3]
            srcLoc = tokens[4]
            # ignore external source code
            if srcLoc.startswith("external/"):
                continue
            # ignore build folder
            if srcLoc.startswith("workdir/"):
                continue
            definitionSet.add(fieldInfo)
            definitionToTypeMap[fieldInfo] = fieldType
            definitionToSourceLocationMap[fieldInfo] = srcLoc
        elif tokens[0] == "touched:":
            fieldInfo = (normalizeTypeParams(tokens[1]), tokens[2])
            touchedByFunction = normalizeTypeParams(tokens[3])
            touchedByFunctionSrcLoc = tokens[4]
            if fieldInfo in excludeSet:
                continue
            if touchedByFunction == "Negative":
                excludeSet.add(fieldInfo)
                if fieldInfo in touchedMap:
                    touchedMap.pop(fieldInfo)
            elif fieldInfo in touchedMap:
                if touchedMap[fieldInfo] != touchedByFunction:
                    excludeSet.add(fieldInfo)
                    touchedMap.pop(fieldInfo)
            else:
                touchedMap[fieldInfo] = touchedByFunction
        else:
            print( "unknown line: " + line)

outputSet = set()
for d in definitionSet:
    if d not in touchedMap:
        continue
    fieldType = definitionToTypeMap[d]
    # ignore some types that are known false+
    if (fieldType.startswith("std::unique_ptr<")
        or fieldType == "std::mutex"
        or "Mutex" in fieldType
        or "union" in fieldType
        or "anonymous namespace" in fieldType
        or "unnamed struct" in fieldType):
        continue
    # ignore some field names that are known false+
    if (d[1] == "mbDisposing"
        or d[1] == "bInDispose"
        or d[1] == "m_bDisposing"
        or d[1].startswith("m_bIn")):
        continue
    srcLoc = definitionToSourceLocationMap[d]
    # ignore some types in the system libraries we somehow pick up
    if srcLoc.startswith(".") or srcLoc.startswith("/") or srcLoc.startswith("lib/"):
        continue
    # part of the URE
    if srcLoc.startswith("include/cppuhelper/"):
        continue
    # on-disk structures
    if srcLoc.startswith("hwpfilter/"):
        continue
    if srcLoc.startswith("include/osl/"):
        continue
    if srcLoc.startswith("include/sal/"):
        continue
    if srcLoc.startswith("sw/source/filter/ww8/ww8struc.hxx"):
        continue
    if srcLoc.startswith("sd/source/filter/ppt/ppt97animations.hxx"):
        continue
    if srcLoc.startswith("lotuswordpro/"):
        continue
    if srcLoc.startswith("include/filter/msfilter/svdfppt.hxx"):
        continue
    if srcLoc.startswith("filter/source/graphicfilter/icgm/chart.hxx"):
        continue
    # most of this code is only compiled on windows, so we don't have decent results
    if srcLoc.startswith("include/svl/svdde.hxx"):
        continue
    touchedByFunction = touchedMap[d]
    outputSet.add((d[0] + " " + d[1] + " " + definitionToTypeMap[d], srcLoc, touchedByFunction))

# sort the results using a "natural order" so sequences like [item1,item2,item10] sort nicely
def natural_sort_key(s, _nsre=re.compile('([0-9]+)')):
    return [int(text) if text.isdigit() else text.lower()
            for text in re.split(_nsre, s)]
# sort by both the source-line and the datatype, so the output file ordering is stable
# when we have multiple items on the same source line
def v_sort_key(v):
    return natural_sort_key(v[1]) + [v[0]]

# sort results by name and line number
tmp1list = sorted(outputSet, key=lambda v: v_sort_key(v))

# print out the results
with open("compilerplugins/clang/fieldcanbelocal.results", "wt") as f:
    for t in tmp1list:
        f.write( t[1] + "\n" )
        f.write( "    " + t[0] + "\n" )
        f.write( "    " + t[2] + "\n" )


