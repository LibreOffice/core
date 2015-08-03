#!/usr/bin/python

import subprocess

a = subprocess.Popen("git grep -P '^typedef\s+.+\w+;' -- \"[!e][!x][!t]*\"", stdout=subprocess.PIPE, shell=True)

with a.stdout as txt:
    for line in txt:
        idx1 = line.rfind(" ")
        typedefName = line[idx1+1 : len(line)-2]
        if typedefName.startswith("*"):
           typedefName = typedefName[1:]
        # ignore anything less than 5 characters, it's probably a parsing error
        if len(typedefName) > 4:
            print typedefName

