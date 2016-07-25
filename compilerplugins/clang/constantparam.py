#!/usr/bin/python

import sys
import re
import io

definitionSet = set()
definitionToSourceLocationMap = dict()
callParamSet = dict()

# clang does not always use exactly the same numbers in the type-parameter vars it generates
# so I need to substitute them to ensure we can match correctly.
normalizeTypeParamsRegex = re.compile(r"type-parameter-\d+-\d+")
def normalizeTypeParams( line ):
    return normalizeTypeParamsRegex.sub("type-parameter-?-?", line)

# reading as binary (since we known it is pure ascii) is much faster than reading as unicode
with io.open("loplugin.constantparam.log", "rb", buffering=1024*1024) as txt:
    for line in txt:
        idx1 = line.find("\t")
        idx2 = line.find("\t",idx1+1)
        idx3 = line.find("\t",idx2+1)
        idx4 = line.find("\t",idx3+1)
        returnType = normalizeTypeParams(line[:idx1])
        nameAndParams = normalizeTypeParams(line[idx1+1:idx2])
        sourceLocation = line[idx2+1:idx3]
        paramName = line[idx3+1:idx4]
        callValue = line[idx4+1:].strip()
        callInfo = (returnType, nameAndParams, paramName)
        if not callInfo in callParamSet:
            callParamSet[callInfo] = set()
        callParamSet[callInfo].add(callValue)
        definitionToSourceLocationMap[callInfo] = sourceLocation

tmp1list = list()
for callInfo, callValues in callParamSet.iteritems():
    nameAndParams = callInfo[1]
    if len(callValues) != 1:
        continue
    if "unknown" in callValues:
        continue
    # ignore anything with only one parameter, normally just setter methods
    if nameAndParams.find(",") == -1:
        continue
    # if it contains anything other than this set, ignore it
    if len(callValues - set(["0", "1", "-1", "nullptr"])) > 0:
        continue
    v0 = callInfo[0] + " " + callInfo[1]
    v1 = callInfo[2] + " " + (",".join(callValues))
    v2 = definitionToSourceLocationMap[callInfo]
    tmp1list.append((v0,v1,v2))

# sort results by filename:lineno
def natural_sort_key(s, _nsre=re.compile('([0-9]+)')):
    return [int(text) if text.isdigit() else text.lower()
            for text in re.split(_nsre, s)]
tmp1list.sort(key=lambda v: natural_sort_key(v[2]))

# print out the results
with open("loplugin.constantparams.report", "wt") as f:
    for v in tmp1list:
        f.write(v[2] + "\n")
        f.write("    " + v[0] + "\n")
        f.write("    " + v[1] + "\n")


