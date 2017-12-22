#!/usr/bin/python
#
# Find potentially unused UNO command entries in SDI files.
#
# Note that this is not foolproof, some extra checking is required because some command names might be
# constructed at runtime.
#

import subprocess

# search for entries in .sdi files that declare UNO/SID commands
a = subprocess.Popen("git grep -P '^\s*\w+Item\s+\w+\s+SID_\w+$' -- *.sdi", stdout=subprocess.PIPE, shell=True)

# parse out the UNO command names
commandSet = list()
with a.stdout as txt:
    for line in txt:
        line = line.strip()
        idx1 = line.find(" ")
        idx2 = line.find(" ", idx1 + 1)
        commandName = line[idx1+1 : idx2].strip()
        sidName = line[idx2+1:].strip()
        commandSet.append((commandName,sidName))

# now check to see if that UNO command is called anywhere in the codebase.
for pair in commandSet:
    commandName = pair[0]
    sidName = pair[1]

    # check to see if that UNO command is called anywhere in the codebase.
    a = subprocess.Popen("git grep -wFn '.uno:" + commandName + "'", stdout=subprocess.PIPE, shell=True)
    cnt = 0
    with a.stdout as txt2:
        for line2 in txt2:
            cnt = cnt + 1
    if cnt > 0: continue

    # check to see if the SID is used programmatically
    foundLines = ""
    a = subprocess.Popen("git grep -wn " + sidName, stdout=subprocess.PIPE, shell=True)
    with a.stdout as txt2:
        for line2 in txt2:
            foundLines = foundLines + line2
    if foundLines.find("ExecuteList") != -1: continue
    if foundLines.find("GetDispatcher()->Execute") != -1: continue
    if foundLines.find("ExecuteScenarioSlot") != -1: continue
    # TODO not sure about this, but let's tackle the easy ones first
    if foundLines.find("Invalidate(") != -1: continue

    # dump any lines that contain the SID, so we can eyeball the results
    print("remove: " + commandName)
    print(foundLines)
    print("----------------------------------------------------------------------------")
