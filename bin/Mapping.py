#!/usr/bin/env python3
#
# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.


import os
import json

screenPath = os.path.join(os.getcwd() , 'workdir','screenshots')
PotPath = os.path.join(os.getcwd() , 'workdir','pot')

UIfiles = []
# screenshot_path -> {ui_path , context}
map = {}

def SearchInText(FilePath , TargetUI,ScreenshotPath):
    
    NextLineIsContext = False
    key = ''
    with open(FilePath , 'r' , encoding='utf-8') as f:
        
        for Line in f:
            if(Line.__contains__('#.')):
                keyid = Line.split(' ')
                key = (keyid[1])[:-1]

            elif(NextLineIsContext):
                if(Line.__contains__("\"")):
                    SecondLine = Line.split("\"")
                else:
                    SecondLine = Line.split("/")


                context = SecondLine[1][0:len(SecondLine[1])] #Remove quotes
                if(len(map[ScreenshotPath]) == 2):
                    map[ScreenshotPath][1].append((context , key))
                else:
                    ContextKey = []
                    ContextKey.append((context , key))
                    map[ScreenshotPath].append(ContextKey)
            
                NextLineIsContext = False

            elif Line.find('/' + TargetUI.lower()) !=-1:
                UIpath = Line[3:-1]
                temp = UIpath.split(':') #Ignore line number
                UIpath = temp[0]
                if(ScreenshotPath not in map.keys()):
                    UIPathWithContextKey = []
                    UIPathWithContextKey.append(UIpath)
                    map[ScreenshotPath]= UIPathWithContextKey

                NextLineIsContext = True
                

    
    

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
        UIfilename = files[0][0:-3] + 'ui'
        #Remove 'png' from the screenshot and append 'ui'
        UIfiles.append(UIfilename)
        # Add in UIfiles
        print(UIfilename)
        screenshotPath = os.path.join(dirpath,files[0])
        #Screenshot location -
        # is just dirpath + screenshot name
        # Pass UIfilename to be searched in the pot file
        if(screenshotPath.__contains__('core\\')):
            screenrelative = screenshotPath.split('core\\')
        else:
            screenrelative = screenshotPath.split('core/')
        #make it relative
        
        FindInPot(UIfilename ,screenrelative[1])

#Save map to json format in txt file
txtfilepath = os.path.join(os.getcwd() , 'workdir','screenshots','ScreenshotJson.json')
with open( txtfilepath, 'w') as outfile :
    json.dump(map , outfile)
outfile.close


txtfilepath = os.path.join(os.getcwd() , 'workdir','screenshots','ScreenshotJsonWithUnitID.json')
with open( txtfilepath, 'w') as outfile :
    json.dump(map , outfile)
outfile.close