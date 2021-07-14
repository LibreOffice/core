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
#The script is used to delete all the screenshots
# and post the updated screenshots to Weblate and assign their unit ids .

import getopt
import os
import json
import sys
import time
import requests
import subprocess
import platform

from requests.models import ChunkedEncodingError
from requests.api import head

from requests.sessions import Request, Session

ScreenshotsLink = 'https://weblate-gsoc.documentfoundation.org/api/screenshots/'
idstodelete = []
argumentList = sys.argv[1:]
short_options = "c:"
long_options = ["config"]
apiKey = ''
CurrentUploadedScreenshots = {}
NextPage = False
NextPageUrl = ''
notaddedunits = []
exceptionsthrown = []
currentScreenshots = {}
with open (os.path.join(os.getcwd() , 'workdir' , 'screenshots' , 'ScreenshotJsonWithUnitID.json' ) ) as Json_File:
    screenshotsWithUnitId = json.load(Json_File)

try:
    args , vals  = getopt.getopt(argumentList, short_options ,long_options)
except getopt.error as err:
    print(str(err))
    args = []

for argument , currval in args:
    if(argument in ["-c","--config"]):
        ConfigFilePath = currval

    else:
        print(argument)
if(args != []):
    try:
        configfile = open(ConfigFilePath , 'r')
        apikeyWithMainlink = (configfile.read() ).split('\n')
        apiKey = apikeyWithMainlink[0]
        mainLink =os.path.join( apikeyWithMainlink[1].rstrip() , 'screenshots/' )
    except FileNotFoundError:
        print("Wrong file or file path")
        exit()


CurrentscreenshotsRequest = requests.get(ScreenshotsLink , headers={"Authorization" :  apiKey})

while(True):
    if(NextPage):
        CurrentscreenshotsResponse = NextPageUrl.content
    else:
        CurrentscreenshotsResponse = CurrentscreenshotsRequest.content

    Screenshots = json.loads(CurrentscreenshotsResponse)
    ScreenshotsResults = Screenshots['results']

    for Screenshot in ScreenshotsResults :
        CurrentUploadedScreenshots[Screenshot['name']] = Screenshot
        screenshoturl = Screenshot['url'].split('/')
        screenshotid = screenshoturl[5]
        idstodelete.append(screenshoturl[5])

    if(Screenshots['next'] != None):
        NextPage = True
        NextPageUrl = requests.get(Screenshots['next'] , headers={"Authorization" :  apiKey})
    else:
        NextPage = False
        break
    print( int((NextPageUrl.headers)['X-RateLimit-Remaining']) )
    if( (int((CurrentscreenshotsRequest.headers)['X-RateLimit-Remaining']) < 80) or int((NextPageUrl.headers)['X-RateLimit-Remaining']) < 80 ):
        timewaiting = (CurrentscreenshotsRequest.headers)['X-RateLimit-Reset']
        print('program will Wait ' , timewaiting , 'seconds not to blocked due to requests limit')
        time.sleep(int((CurrentscreenshotsRequest.headers)['X-RateLimit-Reset']))

for id in idstodelete:
    print('screenshot '+id + ' Deleted')
    requests.delete(ScreenshotsLink + '/' + id + '/',headers={"Authorization" :  apiKey} )

for screenshotpath in screenshotsWithUnitId.keys():

    screenshotname = os.path.basename(screenshotpath)
    projectslug = 'libo_ui-master'
    for ui in screenshotsWithUnitId[screenshotpath].keys():
        componentslug = (ui.split('/') )[0] + 'messages'
        languagecode = 'en'
        parameters = {
        'name' : screenshotname ,'project_slug' :projectslug,
        'component_slug' : componentslug , 'language_code' : languagecode }

        fileparameter = {'image' : open(os.path.join(os.getcwd() , screenshotpath) , 'rb')}
        uilist = []
        try:
            response = requests.post(mainLink ,
            files = fileparameter,
            data = parameters,
            headers = {"Authorization" : apiKey},
            timeout = 5
            )
            #print('ratelimit remaining :' + (response.headers)['X-RateLimit-Remaining'] + 'time remaining ' +(CurrentscreenshotsRequest.headers)['X-RateLimit-Reset'])

            if( int((response.headers)['X-RateLimit-Remaining']) < 80 ):
                timewaiting = (response.headers)['X-RateLimit-Reset']
                print('screenshtpost program will Wait ' , timewaiting , 'seconds not to blocked due to requests limit')
                time.sleep(int((response.headers)['X-RateLimit-Reset']))

            response = response.content
            screenshotresponse = json.loads(response)
            screenshotId = (screenshotresponse['url']).rsplit('/', 1)[0].rsplit('/',1)[1]
            print('Screenshot number:' , screenshotId , ' has been posted')
            uilist = screenshotsWithUnitId[screenshotpath][ui]
            currentScreenshots[screenshotname] = [screenshotId]
            imagedata = subprocess.run(['identify',  '-verbose' , screenshotpath  , 'grep' , 'signature'] , capture_output=True )
            imagedata = imagedata.stdout.decode("utf-8")
            imagedata = imagedata.split(" ")
            imagesignature = imagedata[imagedata.index('signature:') + 1]
            imagesignature = imagesignature[0:-1]
            currentScreenshots[screenshotname].append(imagesignature)
        except Exception as e:
            exceptionsthrown.append(e)
            print(e)
            print(parameters)
            print(response)


        for i in range(len (uilist)):
            url = mainLink + screenshotId + '/units/'
            try:
                unitid = {'unit_id' : uilist[i][2]}
                responseToAssociation = requests.post(url , data = unitid,
                headers = {"Authorization" :  apiKey},timeout = 10
                )
                print('ratelimit remaining :' + (responseToAssociation.headers)['X-RateLimit-Remaining'] + 'time remaining ' +(responseToAssociation.headers)['X-RateLimit-Reset'])

                if( int((responseToAssociation.headers)['X-RateLimit-Remaining']) < 80 ):
                    timewaiting = (responseToAssociation.headers)['X-RateLimit-Reset']
                    print('assignment program will Wait ' , timewaiting , 'seconds not to blocked due to requests limit')
                    time.sleep(int((responseToAssociation.headers)['X-RateLimit-Reset']))

                print('Assigned screenshot no.' , screenshotId ,' with with unit of id :', uilist[i][2] )
                currentScreenshots[screenshotname].append(uilist[i])
            except IndexError:
                print ('The Unit id is missing')
                print(url)
            except Exception as e:
                print(e,url)
                notaddedunits.append(uilist)

with open( os.path.join(os.getcwd() , 'workdir','screenshots','currentScreenshots.json'), 'w') as outfile :
    json.dump(currentScreenshots , outfile)
outfile.close
#used it to log the exceptions to be able to look at them after the program finish

with open( os.path.join(os.getcwd() , 'workdir','screenshots','notadded.json'), 'w') as outfile :
    json.dump(notaddedunits , outfile)
outfile.close