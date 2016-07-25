#!/usr/bin/python

import sys
import re
import io

definitionToSourceLocationMap = dict() # dict of tuple(parentClass, fieldName) to sourceLocation
fieldAssignDict = dict() # dict of tuple(parentClass, fieldName) to (set of values)

# clang does not always use exactly the same numbers in the type-parameter vars it generates
# so I need to substitute them to ensure we can match correctly.
normalizeTypeParamsRegex = re.compile(r"type-parameter-\d+-\d+")
def normalizeTypeParams( line ):
    return normalizeTypeParamsRegex.sub("type-parameter-?-?", line)

# reading as binary (since we known it is pure ascii) is much faster than reading as unicode
with io.open("loplugin.singlevalfields.log", "rb", buffering=1024*1024) as txt:
    for line in txt:
        if line.startswith("defn:\t"):
            idx1 = line.find("\t")
            idx2 = line.find("\t",idx1+1)
            idx3 = line.find("\t",idx2+1)
            parentClass = normalizeTypeParams(line[idx1+1:idx2])
            fieldName = normalizeTypeParams(line[idx2+1:idx3])
            sourceLocation = line[idx3+1:].strip()
            fieldInfo = (parentClass, fieldName)
            definitionToSourceLocationMap[fieldInfo] = sourceLocation
        elif line.startswith("asgn:\t"):
            idx1 = line.find("\t")
            idx2 = line.find("\t",idx1+1)
            idx3 = line.find("\t",idx2+1)
            parentClass = normalizeTypeParams(line[idx1+1:idx2])
            fieldName = normalizeTypeParams(line[idx2+1:idx3])
            assignValue = line[idx3+1:].strip()
            fieldInfo = (parentClass, fieldName)
            if not fieldInfo in fieldAssignDict:
                fieldAssignDict[fieldInfo] = set()
            fieldAssignDict[fieldInfo].add(assignValue)

tmp1list = list()
for fieldInfo, assignValues in fieldAssignDict.iteritems():
    if len(assignValues) != 1:
        continue
    if "?" in assignValues:
        continue
    # if it contains anything other than this set, ignore it
    if len(assignValues - set(["0", "1", "-1", "nullptr"])) > 0:
        continue
    # ignore things which are locally declared but are actually redeclarations of things from 3rd party code
    parentClass = fieldInfo[0]
    if parentClass == "_mwmhints":
        continue
    # ignore things which are representations of on-disk structures
    if parentClass in ["SEPr", "WW8Dop", ]:
        continue
    v0 = fieldInfo[0] + " " + fieldInfo[1]
    v1 = (",".join(assignValues))
    v2 = ""
    if fieldInfo in definitionToSourceLocationMap:
        v2 = definitionToSourceLocationMap[fieldInfo]
    tmp1list.append((v0,v1,v2))

# sort results by filename:lineno
def natural_sort_key(s, _nsre=re.compile('([0-9]+)')):
    return [int(text) if text.isdigit() else text.lower()
            for text in re.split(_nsre, s)]
tmp1list.sort(key=lambda v: natural_sort_key(v[2]))

# print out the results
with open("loplugin.singlevalfields.report", "wt") as f:
    for v in tmp1list:
        f.write(v[2] + "\n")
        f.write("    " + v[0] + "\n")
        f.write("    " + v[1] + "\n")


