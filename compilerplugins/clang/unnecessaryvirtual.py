#!/usr/bin/python

import sys
import io

definitionSet = set()
overridingSet = set()


with io.open("loplugin.unnecessaryvirtual.log", "rb", buffering=1024*1024) as txt:
    for line in txt:
    
        if line.startswith("definition:\t"):
            idx1 = line.find("\t")
            clazzName = line[idx1+1 : len(line)-1]
            definitionSet.add(clazzName)
            
        elif line.startswith("overriding:\t"):
            idx1 = line.find("\t")
            clazzName = line[idx1+1 : len(line)-1]
            overridingSet.add(clazzName)
            
with open("loplugin.unnecessaryvirtual.report", "wt") as f:
    for clazz in sorted(definitionSet - overridingSet):
        # external code
        if clazz.startswith("std::"): continue
        if clazz.startswith("icu_"): continue
        if clazz.startswith("__cxx"): continue
        # windows-specific stuff
        if clazz.startswith("canvas::"): continue
        if clazz.startswith("psp::PrinterInfoManager"): continue
        # some test magic
        if clazz.startswith("apitest::"): continue
        f.write(clazz + "\n")
    # add an empty line at the end to make it easier for the removevirtuals plugin to mmap() the output file 
    f.write("\n")

