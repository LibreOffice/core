#!/usr/bin/python3

# Look for headers inside include/ that can be moved into their respective modules.
# Not 100% accurate

import subprocess

headerSet = set()
a = subprocess.Popen("git ls-files include/", stdout=subprocess.PIPE, shell=True)
with a.stdout as txt:
    for line in txt:
        header = line[8:].strip()
        if b"README" in header: continue
        if header == b"version.hrc": continue
        # ignore URE headers
        if header.startswith(b"IwyuFilter_include.yaml"): continue
        if header.startswith(b"cppu/"): continue
        if header.startswith(b"cppuhelper/"): continue
        if header.startswith(b"osl/"): continue
        if header.startswith(b"sal/"): continue
        if header.startswith(b"salhelper/"): continue
        if header.startswith(b"uno/"): continue
        headerSet.add(header)

headerSetUnused = headerSet.copy()
headerSetOnlyInOwnModule = headerSet.copy()
a = subprocess.Popen("git grep '^#include <'", stdout=subprocess.PIPE, shell=True)
with a.stdout as txt:
    for line in txt:
        idx1 = line.find(b"#include <")
        idx2 = line.find(b">", idx1 + 10)
        include = line[idx1 + 10 : idx2]
        headerSetUnused.discard(include)
        #
        idx1 = line.find(b"/")
        includedFromModule = line[0 : idx1]
        idx1 = include.find(b"/")
        module = include[0 : idx1]
        if module != includedFromModule:
            headerSetOnlyInOwnModule.discard(include)

print("completely unused")
print("----------------------------")
for x in sorted(headerSetUnused):
    print(x)
print("")
print("only used in own module")
print("----------------------------")
for x in sorted(headerSetOnlyInOwnModule):
    print(x)
