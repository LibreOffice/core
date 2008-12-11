#!/usr/bin/python

#*************************************************************************
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#***********************************************************************/

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