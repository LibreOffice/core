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
#The script GET unit id from the Weblate API and assign them to the screenshots
#Note : Mapping.py should be run first

import os
import json
import requests
import time
import sys
import getopt

mainLink = 'https://weblate-gsoc.documentfoundation.org/api/translations/'
txtfilepath = os.path.join(os.getcwd() , 'workdir' , 'screenshots' , 'ScreenshotJson.json' )
argumentList = sys.argv[1:]
short_options = "c:"
long_options = ["config"]
map = {}
keyidToUnitID = {}
cnt =0
apiKey = ''
try:
    args , vals  = getopt.getopt(argumentList, short_options ,long_options)
except getopt.error as err:
    print(str(err))
    args = []

for argument , currval in args:
    if(argument in ["-c","--config"]):
        ConfigFilePath = currval


if(args != []):
    try:
        configfile = open(ConfigFilePath , 'r')
        apikeyWithMainlink = (configfile.read() ).split('\n')
        apiKey = apikeyWithMainlink[0].rstrip()
        mainLink =os.path.join( apikeyWithMainlink[1].rstrip() , 'translations/')
    except FileNotFoundError:
        print("Wrong file or file path")
        exit()

# Convert Map < screenshotPath -> (uiPath , context)Pair > in json to dict
try:
    with open (os.path.join(os.getcwd() ,'workdir' , 'screenshots' , 'ScreenshotJson.json') ) as Json_File:
        map = json.load(Json_File)
except FileNotFoundError:
    print('Try to run \'Mapping.py\' script first to generate screenshotjson file')
    exit()


for screenshotLocation in map.keys():
    cnt += 1
    for UIPath in map[screenshotLocation]:
        componentSlug = UIPath.split('/')[0] + ('messages/')
        ProjectSlug = 'libo_ui-master/'
        language = 'en/'
        Location = 'location:' + UIPath
        query = {'q' : Location}

        request = requests.get(mainLink + ProjectSlug +
        componentSlug + language + 'units/',params = query ,
        headers = {"Authorization" :  apiKey},
        timeout = 5)
        nextPageUrl = ''
        ContainNextPage = False

        if(int((request.headers)['X-RateLimit-Remaining'] )< 80):
            timewaiting = (request.headers)['X-RateLimit-Reset']
            print('program will Wait ' , timewaiting , 'seconds not to blocked due to requests limit')
            time.sleep((request.headers)['X-RateLimit-Reset'])

        while(True):

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
                nextPageUrl = requests.get(units['next'] ,
                headers = {"Authorization" :  apiKey},
                timeout = 5)

            else:
                break

            if(int((request.headers)['X-RateLimit-Remaining']) < 80 or int((nextPageUrl.headers)['X-RateLimit-Remaining']) < 80 ):
                timewaiting = (request.headers)['X-RateLimit-Reset']
                print('program will Wait ' , timewaiting , 'seconds not to blocked due to requests limit')
                time.sleep((request.headers)['X-RateLimit-Reset'])

    print(len(map.keys()) - cnt,"screenshot remaining" )

#Save the mapping of keyid of context to unit idd .
txtfilepath = os.path.join(os.getcwd() , 'workdir','screenshots','KeyIdtoUnitID.json')
with open( txtfilepath, 'w') as outfile :
    json.dump(keyidToUnitID , outfile)
outfile.close


#put the unit id of context in the map and save them .

for screenshotLocation in map.keys():

    for ui in map[screenshotLocation]:
        for l in map[screenshotLocation][ui]:
            try:
                l.append(keyidToUnitID[l[1]])
            except KeyError :
                print('unit : ' + l[1] + ' is not found')


txtfilepath = os.path.join(os.getcwd() , 'workdir','screenshots','ScreenshotJsonWithUnitID.json')
with open( txtfilepath, 'w') as outfile :
    json.dump(map , outfile)
outfile.close