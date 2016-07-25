#!/usr/bin/python

import sys

instantiatedSet = set()
definitionSet = set()
parentChildDict = {}
definitionToFileDict = {}

with open("loplugin.mergeclasses.log") as txt:
    for line in txt:
    
        if line.startswith("instantiated:\t"):
            idx1 = line.find("\t")
            clazzName = line[idx1+1 : len(line)-1]
            if (clazzName.startswith("const ")):
                clazzName = clazzName[6:]
            if (clazzName.startswith("class ")):
                clazzName = clazzName[6:]
            if (clazzName.endswith(" &")):
                clazzName = clazzName[:len(clazzName)-3]
            instantiatedSet.add(clazzName)
            
        elif line.startswith("definition:\t"):
            idx1 = line.find("\t")
            idx2 = line.find("\t", idx1+1)
            clazzName = line[idx1+1 : idx2]
            # the +2 is so we skip the leading /
            fileName  = line[idx2+2 : len(line)-1]
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
            
def extractModuleName(clazz):
    filename = definitionToFileDict[clazz]
    if filename.startswith("include/"):
        filename = filename[8:]
    idx = filename.find("/")
    return filename[:idx]

with open("loplugin.mergeclasses.report", "wt") as f:
for clazz in sorted(definitionSet - instantiatedSet):
    # find uninstantiated classes without any subclasses
    if (not(clazz in parentChildDict)) or (len(parentChildDict[clazz]) != 1):
        continue
    # exclude some common false positives
    a = ['Dialog', 'Dlg', 'com::sun', 'Base']
    if any(x in clazz for x in a):
        continue
    # ignore base class that contain the word "mutex", they are normally there to
    # help with the WeakComponentImpl template magic
    if ("mutex" in clazz) or ("Mutex" in clazz):
        continue
    otherclazz = next(iter(parentChildDict[clazz]))
    # exclude combinations that span modules because we often use those to make cross-module dependencies more manageable.
    if extractModuleName(clazz) != extractModuleName(otherclazz):
        continue
    f.write( "merge" + clazz + "with" + otherclazz + "\n" )

