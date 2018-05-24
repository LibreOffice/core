#!/usr/bin/python
#
# Look for slot IDs (SIDs) that are mentioned in SDI files in a module, but not processed in any of the state methods
# of that module
#
# Note that this is not foolproof, some extra checking is required because some command names might be
# constructed at runtime.
#

import subprocess

# search for entries in .sdi files that declare UNO/SID commands
a = subprocess.Popen("cd sw && git grep -hP '(FN_)|(SID_)' -- *.sdi", stdout=subprocess.PIPE, shell=True)

# parse out the UNO command names
commandSet = list()
with a.stdout as txt:
    for line in txt:
        if "Item" in line: continue
        if "SwFormat" in line: continue
        if "//" in line: line = line[ : line.find("//") ]
        if "[" in line: line = line[ : line.find("[") ]
        line = line.strip()
        if not line: continue
        # print line
        commandSet.append(line)

# now check to see if that SID is processed in the C++ code
for sidName in commandSet:

    a = subprocess.Popen("cd sw && git grep -wn " + sidName + " -- *.cxx", stdout=subprocess.PIPE, shell=True)
    cnt = 0
    foundLines = ""
    with a.stdout as txt2:
        for line2 in txt2:
            cnt = cnt + 1
            foundLines = foundLines + line2
    if cnt > 0: continue

    # dump any lines that contain the SID, so we can eyeball the results
    foundLines = ""
    a = subprocess.Popen("git grep -wn " + sidName + " sw/", stdout=subprocess.PIPE, shell=True)
    with a.stdout as txt2:
        for line2 in txt2:
            foundLines = foundLines + line2
    print("remove: " + sidName)
    print(foundLines)
    print("----------------------------------------------------------------------------")
