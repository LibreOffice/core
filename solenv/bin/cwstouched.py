#!/usr/bin/python

#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************

import os
import sys
import string
from os import path

def getCurrPath():
    currPath = sys.path[0] or os.getcwd()
    currPath = path.abspath(currPath)
    return currPath
    
def getCwsWorkStamp():
    cwsWorkStamp=os.getenv('CWS_WORK_STAMP')
    
    if not cwsWorkStamp:
        currPath=getCurrPath()
        
        os.chdir(os.getenv('SOLARENV'))
        
        (input, output) = os.popen4("svn info")
        
        for outline in output.readlines():
            if outline.startswith("URL:"):
                cwsWorkStamp = outline[outline.index("svn.services"):outline.index("solenv")-1]
                cwsWorkStamp = cwsWorkStamp[cwsWorkStamp.rfind("/")+1:len(cwsWorkStamp)]
                break
                
        os.putenv("CWS_WORK_STAMP",cwsWorkStamp);
        os.chdir(currPath)
    
    return string.strip(cwsWorkStamp)
    
def getMinor(cwsWorkStamp):
    minor = os.getenv('UPDMINOR')
    
    if not minor:
        if (os.getenv('OSTYPE') == "cygwin"):
            bash=os.getenv("SHELL")
            (input, output) = os.popen4("cygpath -w "+bash)
            winbash=string.strip(output.readlines()[0])
            cws=winbash+" -c 'cws  query -c "+cwsWorkStamp+" current'"
        else:
            cws="cws query -c "+cwsWorkStamp+" current"

        (input, output) = os.popen4(cws)
        
        found=0
        for outline in output.readlines():
            if found:
                minor=outline
                break
            elif outline.find("Current milestone:") != -1:
                found=1
    
    return string.strip(minor)
    
    
workstamp = os.getenv('WORK_STAMP')
solenv= os.getenv('SOLARENV')
cwsWorkStamp=getCwsWorkStamp()
minor = getMinor(cwsWorkStamp)

oldWorkStamp = workstamp + "_" + minor
diff="svn diff --summarize --old=svn://svn.services.openoffice.org/ooo/tags/"+oldWorkStamp+" --new=svn://svn.services.openoffice.org/ooo/cws/"+cwsWorkStamp

modules=[]
(input, output) = os.popen4(diff)

for outline in output.readlines():
    if outline.find("svn://svn.services.openoffice.org"):
        index = outline.index(oldWorkStamp)+len(oldWorkStamp)+1
        newModule=""
        if outline.find("/",index) != -1:
            # seems to be a file
            newModule=string.strip(outline[index:outline.index("/",index)])
        else:
            #seems to be a folder
            if len(outline[index:]) > 0:
                newModule=string.strip(outline[index:])
        if newModule != "" and not modules.count(newModule): 
            modules.append(newModule)

for module in modules:
    print module
