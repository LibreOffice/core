#!/usr/bin/python

import re
import sys

definitionSet = set()
inheritFromSet = set()
definitionToFileDict = {}

with open("loplugin.finalclasses.log") as txt:
    for line in txt:
        tokens = line.strip().split("\t")
    
        if len(tokens) == 1:
            pass

        elif tokens[0] == "definition:":
            clazzName = tokens[1]
            # the 1.. is so we skip the leading /
            fileName  = tokens[2][1:]
            definitionSet.add(clazzName)
            definitionToFileDict[clazzName] = fileName
            
        elif tokens[0] == "inherited-from:":
            parent = tokens[1]
            if (parent.startswith("class ")):
                parent = parent[6:]
            elif (parent.startswith("struct ")):
                parent = parent[7:]
            inheritFromSet.add(parent);

tmpset = set()
for clazz in sorted(definitionSet - inheritFromSet):
    tmpset.add((clazz, definitionToFileDict[clazz]))

# sort the results using a "natural order" so sequences like [item1,item2,item10] sort nicely
def natural_sort_key(s, _nsre=re.compile('([0-9]+)')):
    return [int(text) if text.isdigit() else text.lower()
            for text in re.split(_nsre, s)]
def sort_set_by_natural_key(s):
    return sorted(s, key=lambda v: natural_sort_key(v[1]))

# print output, sorted by name and line number
with open("compilerplugins/clang/finalclasses.results", "wt") as f:
    for t in sort_set_by_natural_key(tmpset):
        f.write(t[1] + "\n")
        f.write("    " + t[0] + "\n")

