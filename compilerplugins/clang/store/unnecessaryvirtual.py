#!/usr/bin/python

import sys
import io

definitionSet = set()
overridingSet = set()


with io.open(sys.argv[1], "rb", buffering=1024*1024) as txt:
    for line in txt:
    
        if line.startswith("definition:\t"):
            idx1 = line.find("\t")
            clazzName = line[idx1+1 : len(line)-1]
            definitionSet.add(clazzName)
            
        elif line.startswith("overriding:\t"):
            idx1 = line.find("\t")
            clazzName = line[idx1+1 : len(line)-1]
            overridingSet.add(clazzName)
            
for clazz in sorted(definitionSet - overridingSet):
    print clazz

# add an empty line at the end to make it easier for the removevirtuals plugin to mmap() the output file 
print

