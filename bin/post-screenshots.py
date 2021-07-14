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
import platform
mainLink = 'https://weblate-gsoc.documentfoundation.org/api/screenshots/'
with open (os.path.join(os.getcwd() ,'workdir' , 'screenshots' , 'ScreenshotJsonWithUnitID.json') ) as Json_File:
    screenshotsWithUnitId = json.load(Json_File)
haveApiKey = True
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
        haveApiKey = False
        break


linux = False
if(platform.system()=='Linux' ):
    linux = True

for screenshotpath in screenshotsWithUnitId.keys():
    
    screenshotfile = open(os.path.join(os.getcwd() , screenshotpath) ,'rb')
    screenshotname = os.path.basename(screenshotpath)
    projectslug = 'libo_ui-master'
    
    if(not linux):
        componentslug = (screenshotsWithUnitId[screenshotpath][0].split('/') )[0] + 'messages'
    else:
        componentslug = (screenshotsWithUnitId[screenshotpath][0].split("\"") )[0] + 'messages'
    languagecode = 'en'
    param = {'image' : screenshotfile ,
    'name' : screenshotname ,'project_slug' :projectslug,
    'component_slug' : componentslug , 'language_code' : languagecode }

    if(haveApiKey):
        response = requests.post(mainLink ,data = param,
        headers = {"Authorization" : "Bearer " + apiKey},
        timeout = 5)
    else:
        response = requests.post(mainLink ,data = param,
        timeout = 5)
    print(response)
    
    screenshotId = (response['url']).rsplit('/', 1)[0].rsplit('/',1)[1]

    for units in screenshotpath[1]:
        url = mainLink + '/' + screenshotId + '/units/'
        unitid = {'unit_id' : units[2]}
        if(haveApiKey):
            responseToAssociation = requests.post(url , params = unitid,
            headers = {"Authorization" : "Bearer " + apiKey},timeout = 5
        )
        else:
            responseToAssociation = requests.post(url , params = unitid,
            timeout = 5)