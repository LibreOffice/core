"""
Copyright 2011 Ederson Machado de Lima (edersonn@gmail.com). All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are
permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this list of
        conditions and the following disclaimer.

    2. Redistributions in binary form must reproduce the above copyright notice, this list
        of conditions and the following disclaimer in the documentation and/or other materials
        provided with the distribution.

THIS SOFTWARE IS PROVIDED BY Ederson Machado de Lima (edersonn@gmail.com) ''AS IS'' AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Ederson Machado (edersonn@gmail.com) OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
                       SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
                                                                         NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are those of the
authors and should not be interpreted as representing official policies, either expressed
or implied, of Ederson Machado (edersonn@gmail.com).
"""



"""
    Important 1: There is a bug on ibtool when you add a 'Segment Control View' and then remove it... don't do it :)
    Important 2: See this video to know how this script work with more details: http://www.youtube.com/watch?v=cF1Rf02QvZQ
"""

import sys
import subprocess
import getopt
import os.path

options, idioms = getopt.getopt(sys.argv[1:], "", ["mainStoryboard=", "mainIdiom="])

#retrieve parameters
for option1, option2 in options:
    if option1 == "--mainIdiom":
        mainIdiom = option2
    elif option1 == "--mainStoryboard":
        newMainStoryboard = option2
        oldMainStoryboard = newMainStoryboard.replace(".storyboard", "_old.storyboard")

        #if the old mainStoryboard is not created, create it
        if not os.path.exists(oldMainStoryboard):
            subprocess.call(["cp", newMainStoryboard, oldMainStoryboard])

errorDescription = ""

for idiom in idioms:
    #paths of files for current idiom
    idiomNewStoryboard = newMainStoryboard.replace(mainIdiom + ".lproj", idiom + ".lproj")
    idiomOldStoryboard = idiomNewStoryboard.replace(".storyboard", "_old.storyboard")
    idiomStringsFile = idiomNewStoryboard.replace(".storyboard", ".strings")

    #if the storyboard for the current idiom dont exists, add an error to be show to the user
    if not os.path.exists(idiomNewStoryboard):
        errorDescription += "\n*** You need to add create the '" + idiomNewStoryboard + "' in interface builder*** \n"
    else:
        #copy the storyboard to the old storyboard (current idiom)
        subprocess.call(["cp", idiomNewStoryboard, idiomOldStoryboard])

        #if the strings file (current idiom) dont exists, create it
        if not os.path.exists(idiomStringsFile):
            subprocess.call(["ibtool",
                     "--generate-strings-file", idiomStringsFile,
                     idiomNewStoryboard])

        #generates the incremental storyboard (current idiom)
        if subprocess.call(["ibtool", 
                         "--previous-file", oldMainStoryboard,
                         "--incremental-file", idiomOldStoryboard,
                         "--strings-file", idiomStringsFile,
                         "--localize-incremental",
                         "--write", idiomNewStoryboard,
                                newMainStoryboard]) == 0:

            #generates the strings file (current idiom), to be used on the next build
            subprocess.call(["ibtool",
                         "--generate-strings-file", idiomStringsFile,
                         idiomNewStoryboard])

        #if an error occurred, add an error to be show to the user
        else:
            errorDescription += "\n*** Error while creating the '" + idiomNewStoryboard + "' file*** \n"


#Copy the main storyboard to the old one (main idiom) to be used on the next build
subprocess.call([ "cp", newMainStoryboard, oldMainStoryboard])

#generates the strings file (main idiom), to be used on the next build
subprocess.call(["ibtool",
                 "--generate-strings-file", newMainStoryboard.replace(".storyboard", ".strings"),
                 newMainStoryboard])

#if an error occurred, throws an exception to fail the build process
if errorDescription != "":
    raise Exception("\n" + errorDescription)