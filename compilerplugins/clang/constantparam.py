#!/usr/bin/python

import sys
import re
import io

definitionSet = set()
definitionToSourceLocationMap = dict()
callParamSet = dict()

# things we need to exclude for reasons like :
# - it's a weird template thingy that confuses the plugin
exclusionSet = set([
])

# clang does not always use exactly the same numbers in the type-parameter vars it generates
# so I need to substitute them to ensure we can match correctly.
normalizeTypeParamsRegex = re.compile(r"type-parameter-\d+-\d+")
def normalizeTypeParams( line ):
    return normalizeTypeParamsRegex.sub("type-parameter-?-?", line)

# The parsing here is designed to avoid grabbing stuff which is mixed in from gbuild.
# I have not yet found a way of suppressing the gbuild output.
with io.open(sys.argv[1], "rb", buffering=1024*1024) as txt:
    for line in txt:
            idx1 = line.find("\t")
            idx2 = line.find("\t",idx1+1)
            idx3 = line.find("\t",idx2+1)
            idx4 = line.find("\t",idx3+1)
            returnType = line[:idx1]
            nameAndParams = line[idx1+1:idx2]
            sourceLocation = line[idx2+1:idx3]
            paramName = line[idx3+1:idx4]
            callValue = line[idx4+1:].strip()
            callInfo = (normalizeTypeParams(returnType), normalizeTypeParams(nameAndParams), paramName)
            if callInfo in callParamSet:
                callParamSet[callInfo].add(callValue)
            else:
                callParamSet[callInfo] = set([callValue])
            definitionToSourceLocationMap[callInfo] = sourceLocation

tmp1set = set()
for callInfo, callValues in callParamSet.iteritems():
    if len(callValues) == 1 and "unknown" not in callValues and ("0" in callValues or "1" in callValues or "nullptr" in callValues):
        v1 = (" ".join(callInfo)) + " " + (",".join(callValues))
        v2 = definitionToSourceLocationMap[callInfo]
        tmp1set.add((v1,v2))

# sort results by name and line number
def natural_sort_key(s, _nsre=re.compile('([0-9]+)')):
    return [int(text) if text.isdigit() else text.lower()
            for text in re.split(_nsre, s)]
tmp1list = sorted(tmp1set, key=lambda v: natural_sort_key(v[1]))

# print out the results
with open("unused.constantparams", "wt") as f:
    for t in tmp1list:
        f.write(t[1] + "\n")
        f.write("    " + t[0] + "\n")


