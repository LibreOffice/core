#!/usr/bin/env python3
#
# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#The script iterate on every screenshot and map them to their UI files
#then maps every UI path to context and keyid

import os
import json

screenPath = os.path.join(os.getcwd() , 'workdir','screenshots')
PotPath = os.path.join(os.getcwd() , 'workdir','pot')

UIfiles = []
# screenshot_path -> {ui_path , context}
map = {}
i =0
keyid= ''
def SearchInText(FilePath , TargetUI,ScreenshotPath):
    twolinescontext = False
    NextLineIsContext = False
    key = ''
    UIpath = ''
    UIPathWithContextKey = {}

    with open(FilePath , 'r' , encoding='utf-8') as f:
        for Line in f:
            if(Line.__contains__('#.') ):
                keyid = Line.split(' ',1)
                if(twolinescontext == True):
                    key =key + '\n'+ (keyid[1])[:-1]
                    twolinescontext = False
                else:
                    key = (keyid[1])[:-1]
                    twolinescontext = True
            elif(NextLineIsContext):
                twolinescontext = False
                SecondLine = Line.split("\"")

                context = SecondLine[1][0:len(SecondLine[1])] #Remove quotes

                map[ScreenshotPath][UIpath].append((context , key))
                NextLineIsContext = False

            elif Line.find('/' + TargetUI.lower()) !=-1:
                twolinescontext = False
                UIpath = Line[3:-1]
                temp = UIpath.split(':') #Ignore line number
                UIpath = temp[0]

                if(ScreenshotPath not in map.keys()):
                    UIPathWithContextKey[UIpath] = []
                    map[ScreenshotPath]=(UIPathWithContextKey)
                else:
                    if(UIpath not in map[ScreenshotPath]):
                        map[ScreenshotPath][(UIpath)] = []

                NextLineIsContext = True
            else:
                twolinescontext = False

def FindInPot(TargetUI ,ScreenshotPath):
    for dirpath , dirnames , files in os.walk(PotPath): #traverse all pot files
        if(len(files) !=0):
            #Search in the text of the pot file with the target UI
            PotFilePath = os.path.join(dirpath ,files[0])
            SearchInText(PotFilePath,TargetUI,ScreenshotPath)

# Traverse all the directories and files in ScreenshotPath

for dirpath , dirnames , files in os.walk(screenPath):
    # If valid file
    if(len(files) != 0):
        i  += 1
        UIfilename = files[0][0:-3] + 'ui'
        #Remove 'png' from the screenshot and append 'ui'
        UIfiles.append(UIfilename)
        # Add in UIfiles
        print(UIfilename)
        screenshotPath = os.path.join(dirpath,files[0])
        #Screenshot location -
        # is just dirpath + screenshot name
        # Pass UIfilename to be searched in the pot file
        screenrelative = os.path.relpath(screenshotPath)
        #make it relative

        FindInPot(UIfilename ,screenrelative)


#Save map to json format in txt file
txtfilepath1 = os.path.join(screenPath , 'ScreenshotJson.json')

with open( txtfilepath1,'w') as outfile :
    json.dump(map,outfile)
outfile.close