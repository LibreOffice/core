#!/usr/bin/python2

# Look for headers inside include/ that can be moved into their respective modules.
# Not 100% accurate

import subprocess
import sys

headerSet = set()
a = subprocess.Popen("git ls-files include/", stdout=subprocess.PIPE, shell=True)
with a.stdout as txt:
    for line in txt:
        header = line[8:].strip();
        if "README" in header: continue
        if header == "version.hrc": continue
        # ignore URE headers
        if header.startswith("IwyuFilter_include.yaml"): continue
        if header.startswith("cppu/"): continue
        if header.startswith("cppuhelper/"): continue
        if header.startswith("osl/"): continue
        if header.startswith("sal/"): continue
        if header.startswith("salhelper/"): continue
        if header.startswith("uno/"): continue
        # these are direct copies of mozilla code
        if header.startswith("onlineupdate/mozilla/"): continue
        headerSet.add(header)

headerSetUnused = headerSet.copy()
headerSetOnlyInOwnModule = headerSet.copy()
a = subprocess.Popen("git grep '^#include <'", stdout=subprocess.PIPE, shell=True)
with a.stdout as txt:
    for line in txt:
        idx1 = line.find("#include <")
        idx2 = line.find(">", idx1 + 10)
        include = line[idx1 + 10 : idx2]
        headerSetUnused.discard(include)
        #
        idx1 = line.find("/")
        includedFromModule = line[0 : idx1]
        idx1 = include.find("/")
        module = include[0 : idx1]
        if module != includedFromModule:
            headerSetOnlyInOwnModule.discard(include)

print "completely unused"
print "----------------------------"
for x in sorted(headerSetUnused):
    print x
print ""
print "only used in own module"
print "----------------------------"
for x in sorted(headerSetOnlyInOwnModule):
    print x
