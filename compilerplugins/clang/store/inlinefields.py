#!/usr/bin/python

import re
import io

definitionToSourceLocationMap = dict() # dict of tuple(parentClass, fieldName) to sourceLocation
definitionSet = set()
excludedSet = set()
deletedInDestructorSet = set()
newedInConstructorSet = set()

# clang does not always use exactly the same numbers in the type-parameter vars it generates
# so I need to substitute them to ensure we can match correctly.
normalizeTypeParamsRegex = re.compile(r"type-parameter-\d+-\d+")
def normalizeTypeParams( line ):
    return normalizeTypeParamsRegex.sub("type-parameter-?-?", line)

# reading as binary (since we known it is pure ascii) is much faster than reading as unicode
with io.open("workdir/loplugin.inlinefields.log", "rb", buffering=1024*1024) as txt:
    for line in txt:
        tokens = line.strip().split("\t")
        if tokens[0] == "definition:":
            parentClass = normalizeTypeParams(tokens[1])
            fieldName = normalizeTypeParams(tokens[2])
            sourceLocation = tokens[3]
            fieldInfo = (parentClass, fieldName)
            definitionSet.add(fieldInfo)
            definitionToSourceLocationMap[fieldInfo] = sourceLocation
        elif tokens[0] == "excluded:":
            parentClass = normalizeTypeParams(tokens[1])
            fieldName = normalizeTypeParams(tokens[2])
            fieldInfo = (parentClass, fieldName)
            excludedSet.add(fieldInfo)
        elif tokens[0] == "deletedInDestructor:":
            parentClass = normalizeTypeParams(tokens[1])
            fieldName = normalizeTypeParams(tokens[2])
            fieldInfo = (parentClass, fieldName)
            deletedInDestructorSet.add(fieldInfo)
        elif tokens[0] == "newedInConstructor:":
            parentClass = normalizeTypeParams(tokens[1])
            fieldName = normalizeTypeParams(tokens[2])
            fieldInfo = (parentClass, fieldName)
            newedInConstructorSet.add(fieldInfo)
        else:
            print( "unknown line: " + line)

tmp1list = list()
for d in definitionSet:
# TODO see comment in InlineFields::VisitCXXDeleteExpr
#    if d in excludedSet or d not in deletedInDestructorSet or d not in newedInConstructorSet:
    if d in excludedSet or d not in newedInConstructorSet:
        continue
    srcLoc = definitionToSourceLocationMap[d]
    tmp1list.append((d[0] + " " + d[1], srcLoc))

# sort results by filename:lineno
def natural_sort_key(s, _nsre=re.compile('([0-9]+)')):
    return [int(text) if text.isdigit() else text.lower()
            for text in re.split(_nsre, s)]
# sort by both the source-line and the datatype, so the output file ordering is stable
# when we have multiple items on the same source line
def v_sort_key(v):
    return natural_sort_key(v[1]) + [v[0]]
tmp1list.sort(key=lambda v: v_sort_key(v))

# print out the results
with open("loplugin.inlinefields.report", "wt") as f:
    for v in tmp1list:
        f.write(v[1] + "\n")
        f.write("    " + v[0] + "\n")


