#!/usr/bin/python

import sys

instantiatedSet = set()
definitionSet = set()
parentChildDict = {}
definitionToFileDict = {}

with open("unnecessarysuperclass.log") as txt:
    for line in txt:
    
        if line.startswith("instantiated:\t"):
            idx1 = line.find("\t")
            clazzName = line[idx1+1 : len(line)-1]
            instantiatedSet.add(clazzName)
            
        elif line.startswith("definition:\t"):
            idx1 = line.find("\t")
            idx2 = line.find("\t", idx1+1)
            clazzName = line[idx1+1 : idx2]
            fileName  = line[idx2+1 : len(line)-1]
            definitionSet.add(clazzName)
            definitionToFileDict[clazzName] = fileName
            
        elif line.startswith("has-subclass:\t"):
            idx1 = line.find("\t")
            idx2 = line.find("\t", idx1+1)
            child  = line[idx1+1 : idx2]
            parent = line[idx2+1 : len(line)-1]
            if (parent.startswith("class ")):
                parent = parent[6:]
            elif (parent.startswith("struct ")):
                parent = parent[7:]
            if (child.startswith("class ")):
                child = child[6:]
            elif (child.startswith("struct ")):
                child = child[7:]
            if (parent not in parentChildDict):
                parentChildDict[parent] = set()
            parentChildDict[parent].add(child)

for clazz in sorted(definitionSet - instantiatedSet):
    # find uninstantiated classes without any subclasses
    if (not(clazz in parentChildDict)) or (len(parentChildDict[clazz]) != 1):
        continue
    # exclude some common false positives
    a = ['Dialog', 'Dlg', 'com::sun', 'Base']
    if any(x in clazz for x in a):
        continue
    print "merge", clazz, "with", next(iter(parentChildDict[clazz]))

