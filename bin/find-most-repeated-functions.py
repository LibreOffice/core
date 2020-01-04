#!/usr/bin/python
#
# Find the top 100 functions that are repeated in multiple .o files, so we can out-of-line those
#
#

import subprocess
from collections import defaultdict

# the odd bash construction here is because some of the .o files returned by find are not object files
# and I don't want xargs to stop when it hits an error
a = subprocess.Popen("find instdir/program/ -name *.so | xargs echo nm --radix=d --size-sort --demangle | bash", stdout=subprocess.PIPE, shell=True)

#xargs sh -c "somecommand || true"

nameDict = defaultdict(int)
with a.stdout as txt:
    for line in txt:
        line = line.strip()
        idx1 = line.find(" ")
        idx2 = line.find(" ", idx1 + 1)
        name = line[idx2:]
        nameDict[name] += 1

sizeDict = defaultdict(set)
for k, v in nameDict.iteritems():
    sizeDict[v].add(k)

cnt = 0
for k in sorted(list(sizeDict), reverse=True):
    print k
    for v in sizeDict[k]:
        print v
    cnt += 1
    if cnt > 100 : break

#first = sorted(list(sizeDict))[-1]
#print first


#include/vcl/ITiledRenderable.hxx
# why is gaLOKPointerMap declared inside this header?
