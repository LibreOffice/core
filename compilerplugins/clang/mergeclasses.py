#!/usr/bin/python2

import sys

instantiatedSet = set()
definitionSet = set()
parentChildDict = {}
definitionToFileDict = {}

with open("workdir/loplugin.mergeclasses.log") as txt:
    for line in txt:
        tokens = line.strip().split("\t")
    
        if len(tokens) == 1:
            pass

        elif tokens[0] == "instantiated:":
            clazzName = tokens[1]
            if (clazzName.startswith("const ")):
                clazzName = clazzName[6:]
            if (clazzName.startswith("class ")):
                clazzName = clazzName[6:]
            if (clazzName.startswith("::")):
                clazzName = clazzName[2:]
            instantiatedSet.add(clazzName)
            
        elif tokens[0] == "definition:":
            clazzName = tokens[1]
            # the 1.. is so we skip the leading /
            fileName  = tokens[2][1:]
            definitionSet.add(clazzName)
            definitionToFileDict[clazzName] = fileName
            
        elif tokens[0] == "has-subclass:":
            child  = tokens[1]
            parent = tokens[2]
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

with open("compilerplugins/clang/mergeclasses.results", "wt") as f:
    # loop over defined, but not instantiated classes
    for clazz in sorted(definitionSet - instantiatedSet):
        if clazz == "svl::IUndoManager": print parentChildDict[clazz]
        # ignore classes without any children, and classes with more than one child
        if (clazz not in parentChildDict) or (len(parentChildDict[clazz]) != 1):
            continue
        # exclude some common false positives
        a = ['Dialog', 'Dlg', 'com::sun']
        if any(x in clazz for x in a):
            continue
        # ignore base class that contain the word "mutex", they are normally there to
        # help with the WeakComponentImpl template magic
        if ("mutex" in clazz) or ("Mutex" in clazz):
            continue
        otherclazz = next(iter(parentChildDict[clazz]))
        if clazz == "svl::IUndoManager": print extractModuleName(clazz)
        if otherclazz == "svl::IUndoManager": print extractModuleName(otherclazz)
        # exclude combinations that span modules because we often use those to make cross-module dependencies more manageable.
        if extractModuleName(clazz) != extractModuleName(otherclazz):
            continue
        f.write( "merge " + clazz + " with " + otherclazz + "\n" )

