#!/usr/bin/python3

import re

definitionSet = set()
inheritFromSet = set()
definitionToFileDict = {}

with open("workdir/loplugin.finalclasses.log") as txt:
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
            inheritFromSet.add(parent)

        else:
            print( "unknown line: " + line)

match_module_inc1 = re.compile(r'^\w+/inc/')
match_module_inc2 = re.compile(r'^\w+/.*/inc/')
tmpset = set()
for clazz in sorted(definitionSet - inheritFromSet):
    file = definitionToFileDict[clazz]
    # ignore classes defined inside compilation units, the compiler knows they are final already
    if (".cxx" in file): continue
    # ignore test and external code
    if ("/qa/" in file): continue
    if (file.startswith("workdir/")): continue
    # We are only really interested in classes that are shared between linkage units, where the compiler
    # is not able to figure out for itself that classes are final.
    if not(file.startswith("include/") or match_module_inc1.match(file) or match_module_inc2.match(file)): continue
    #if not(file.endswith(".hxx")): continue
    # Exclude URE
    if file.startswith("include/com/"): continue
    if file.startswith("include/cppu/"): continue
    if file.startswith("include/cppuhelper/"): continue
    if file.startswith("include/osl/"): continue
    if file.startswith("include/rtl/"): continue
    if file.startswith("include/sal/"): continue
    if file.startswith("include/salhelper/"): continue
    if file.startswith("include/typelib/"): continue
    if file.startswith("include/uno/"): continue
    # some kind of template noise
    if file.startswith("include/unotest/"): continue
    # no point optimising test code
    if file.startswith("include/test/"): continue
    tmpset.add((clazz, file))

# sort the results using a "natural order" so sequences like [item1,item2,item10] sort nicely
def natural_sort_key(s, _nsre=re.compile('([0-9]+)')):
    return [int(text) if text.isdigit() else text.lower()
            for text in re.split(_nsre, s)]
# sort by both the source-line and the datatype, so the output file ordering is stable
# when we have multiple items on the same source line
def v_sort_key(v):
    return natural_sort_key(v[1]) + [v[0]]
def sort_set_by_natural_key(s):
    return sorted(s, key=lambda v: v_sort_key(v))

# print output, sorted by name and line number
with open("compilerplugins/clang/finalclasses.results", "wt") as f:
    for t in sort_set_by_natural_key(tmpset):
        f.write(t[1] + "\n")
        f.write("    " + t[0] + "\n")

