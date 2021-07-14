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
import requests
import time

mainLink = 'https://weblate-gsoc.documentfoundation.org/api/translations/'
txtfilepath = os.path.join(os.getcwd() , 'workdir' , 'screenhots' , 'ScreenshotJson.json' )

while(True):
    apiKeyPath = input('Enter ApiKey Path for authorization if you do not have one type -1: ' )
    if(apiKeyPath != '-1'):
        try:
            apiKeyFile = open(apiKeyPath , 'r')
            apiKey = apiKeyFile.read()
        except FileNotFoundError:
            print("Wrong file or file path")
        else:
            break
    else:
        apiKey = '-1'
        break
map = {}
keyidToUnitID = {}
cnt =0 
# Convert Map < screenshotPath -> (uiPath , context)Pair > in json to dict
with open (os.path.join(os.getcwd() ,'workdir' , 'screenshots' , 'ScreenshotJson.json') ) as Json_File:
    map = json.load(Json_File)


for screenshotLocation in map.keys():
    cnt += 1
    UIPath = map[screenshotLocation][0]
    componentSlug = UIPath.split('/')[0] + ('messages/')
    ProjectSlug = 'libo_ui-master/'
    language = 'en/'
    Location = ' location:' + UIPath
    query = {'q' : Location} 

    if(apiKey != '-1'):
        request = requests.get(mainLink + ProjectSlug +
        componentSlug + language + 'units/',params = query ,
        headers = {"Authorization" : "Bearer " + apiKey},
        timeout = 5)
    else:
        request = requests.get(mainLink + ProjectSlug +
        componentSlug + language + 'units/',params = query ,
        timeout = 5)

    nextPageUrl = ''
    ContainNextPage = False
    time.sleep(1)
     
    while(True):
        time.sleep(1)
        if(ContainNextPage == True):
            response = (nextPageUrl).content
        else:
            response  = request.content

        #save response
        units = json.loads(response)
        UnitsResults = units['results']

        # Iterate over the response and map every keyid of specific context
        # to it's Unit id
        for unit in UnitsResults:
            keyidToUnitID[unit['note']] = unit['id']

        #if there is next page put the api key and request the next page
        if(units['next'] != None):
            ContainNextPage = True
            if(apiKey != '-1'):
                nextPageUrl = requests.get(units['next'] ,
                headers = {"Authorization" : "Bearer " + apiKey},
                timeout = 5)
            else:
                nextPageUrl = requests.get(units['next'] ,
                timeout = 5)
        else:
            break
    print("screenshot no Done" ,cnt)


#Save the mapping of keyid of context to unit idd .
txtfilepath = os.path.join(os.getcwd() , 'workdir','screenshots','KeyIdtoUnitID.json') 
with open( txtfilepath, 'w') as outfile :
    json.dump(keyidToUnitID , outfile)
outfile.close
 
#put the unit id of context in the map and save them .

for screenshotLocation in map.keys():
    try:
        for l in map[screenshotLocation][1]:
            l.append(keyidToUnitID[l[1]])
    except KeyError:
        continue

    
txtfilepath = os.path.join(os.getcwd() , 'workdir','screenshots','ScreenshotJsonWithUnitID.json')
with open( txtfilepath, 'w') as outfile :
    json.dump(map , outfile)
outfile.close 