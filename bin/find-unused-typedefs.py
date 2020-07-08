#!/usr/bin/python2

import subprocess

# find typedefs, excluding the externals folder
a = subprocess.Popen(r"git grep -P 'typedef\s+.+\s+\w+;' -- \"[!e][!x][!t]*\"", stdout=subprocess.PIPE, shell=True)

# parse out the typedef names
typedefSet = set()
with a.stdout as txt:
    for line in txt:
        idx2 = line.rfind(";")
        idx1 = line.rfind(" ", 0, idx2)
        typedefName = line[idx1+1 : idx2]
        if typedefName.startswith("*"):
           typedefName = typedefName[1:]
        # ignore anything less than 5 characters, it's probably a parsing error
        if len(typedefName) < 5: continue
        typedefSet.add(typedefName)

for typedefName in sorted(typedefSet):
    print("checking: " + typedefName)
    a = subprocess.Popen(["git", "grep", "-wn", typedefName], stdout=subprocess.PIPE)
    foundLine2 = ""
    cnt = 0
    with a.stdout as txt2:
        for line2 in txt2:
            cnt = cnt + 1
            foundLine2 += line2
    if cnt == 1:
        print("remove: " + foundLine2)
    elif cnt == 2:
        print("inline: " + foundLine2)

