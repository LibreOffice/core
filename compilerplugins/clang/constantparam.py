#!/usr/bin/python

import sys
import re
import io

callDict = dict()

# clang does not always use exactly the same numbers in the type-parameter vars it generates
# so I need to substitute them to ensure we can match correctly.
normalizeTypeParamsRegex = re.compile(r"type-parameter-\d+-\d+")
def normalizeTypeParams( line ):
    return normalizeTypeParamsRegex.sub("type-parameter-?-?", line)

# reading as binary (since we known it is pure ascii) is much faster than reading as unicode
with io.open("loplugin.constantparam.log", "rb", buffering=1024*1024) as txt:
    for line in txt:
        tokens = line.strip().split("\t")
        returnType = normalizeTypeParams(tokens[0])
        nameAndParams = normalizeTypeParams(tokens[1])
        sourceLocation = tokens[2]
        paramName = tokens[3]
        paramType = normalizeTypeParams(tokens[4])
        callValue = tokens[5]
        callInfo = (returnType, nameAndParams, paramName, paramType, sourceLocation)
        if not callInfo in callDict:
            callDict[callInfo] = set()
        callDict[callInfo].add(callValue)

tmp1list = list()
for callInfo, callValues in callDict.iteritems():
    nameAndParams = callInfo[1]
    if len(callValues) != 1:
        continue
    callValue = next(iter(callValues))
    if "unknown" in callValue:
        continue
    sourceLoc = callInfo[4]
    functionSig = callInfo[0] + " " + callInfo[1]

    # try and ignore setter methods
    if ("," not in nameAndParams) and (("::set" in nameAndParams) or ("::Set" in nameAndParams)):
        continue
    # ignore code that follows a common pattern
    if sourceLoc.startswith("sw/inc/swatrset.hxx"): continue
    if sourceLoc.startswith("sw/inc/format.hxx"): continue
    # template generated code
    if sourceLoc.startswith("include/sax/fshelper.hxx"): continue
    # debug code
    if sourceLoc.startswith("include/oox/dump"): continue
    # part of our binary API
    if sourceLoc.startswith("include/LibreOfficeKit"): continue

    v2 = callInfo[3] + " " + callInfo[2] + " " + callValue
    tmp1list.append((sourceLoc, functionSig, v2))

# sort results by filename:lineno
def natural_sort_key(s, _nsre=re.compile('([0-9]+)')):
    return [int(text) if text.isdigit() else text.lower()
            for text in re.split(_nsre, s)]
tmp1list.sort(key=lambda v: natural_sort_key(v[0]))

# print out the results
with open("loplugin.constantparam.report", "wt") as f:
    for v in tmp1list:
        f.write(v[0] + "\n")
        f.write("    " + v[1] + "\n")
        f.write("    " + v[2] + "\n")


