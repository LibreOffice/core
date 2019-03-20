#!/usr/bin/env python3
import os,shutil
from subprocess import call,PIPE,Popen

ROOT_DIR = os.path.abspath(os.curdir) #to obtain the path

p=ROOT_DIR.split("/")[:5] #to split the path till /src/libreoffice/
k="/".join(p)
r="/".join(p)+"/solenv/clang-format/blacklist" #to acces the blacklist txt file

qbfile = open(r, "r")  #to open blacklist txt file


for aline in qbfile: #to loop through the blacklist txt file
    v=k+'/'+aline    #to attach the /src/libreoffice/ to file path in balcklist.txt file (i.e:/src/libreoffice/xmlsecurity/source/gpg/CipherContext.cxx)
    l = ['clang-format', v]
    call(l, stdout=open('demofile.txt', 'w'))  #to call clang-format on the file in blacklist.txt file and send o/p to demofile.txt
    p1 = Popen(["diff", "-u", "-s", 'demofile.txt', v], stdout=PIPE) #diff to compare clang-format file and the original file in blacklist.txt file to see whether formatting is required. and PIPE the o/p
    p2 = Popen(["grep", "^+"], stdin=p1.stdout, stdout=PIPE) #to grep the o/p from diff to find the no.of lines of change
    p3 = Popen(["wc", "-l"], stdin=p2.stdout, stdout=PIPE) #to count the no.of changed lines
    add_lines = int(p3.stdout.read()) #save the PIPED out from p3 into add_lines.
    if(add_lines>0): #if there's change in the files and require formatting then
        fileA = open('demofile.txt', 'rb')#open demofile.txt
        fileB = open(v, 'wb')
        shutil.copyfileobj(fileA, fileB) #to copy the clang-formatted file into the original file
        fileA.close()
        fileB.close()
