""" The script is used to delete all the screenshots
    and post the updated screenshots to Weblate and assign their unit ids ."""
#!/usr/bin/env python3
#
# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import getopt
import os
import json
import sys
import time
import subprocess
import requests
from requests.models import ChunkedEncodingError

SCREENSHOTSLINK = 'https://weblate-gsoc.documentfoundation.org/api/screenshots/'
IDS_TO_DELETE = []
ARGUMENT_LIST = sys.argv[1:]
SHORT_OPTIONS = "c:"
LONG_OPTIONS = ["config="]
API_KEY = ''
PROJECT_SLUG = 'libo_ui-master'
CURRENT_UPLOADED_SCREENSHOTS = {}
CURRENT_SCREENSHOTS = {}
NEXT_PAGE = False
NOT_ADDED_UNITS = []
EXCEPTIONS_THROWN = []

def rate_limit_exceeded(time_to_wait):
    """Wait the RateLimit reset"""
    print('program will Wait ', time_to_wait, 'seconds not to blocked due to requests limit')
    time.sleep(time_to_wait)

def get_signature(screenshot_path):
    """Achieve image signature by running imagemagick in the terminal"""
    imagedata = subprocess.run(['identify', '-verbose', screenshot_path]
                               , capture_output=True
                               , check=True)
    imagedata = imagedata.stdout.decode("utf-8")
    imagedata = imagedata.split(" ")
    image_signature = imagedata[imagedata.index('signature:') + 1]
    image_signature = image_signature[0:-1]
    return image_signature


def fetch_api_token(config_file_path):
    """Extract the Api_key and the link from the config file"""
    global API_KEY, MAIN_LINK
    config_file_path = os.path.expanduser(config_file_path)
    configfile = open(config_file_path, 'r')
    for line in configfile:
        if line.__contains__('URL'):
            line = line.split('=')[1]
            line = line.strip()
            MAIN_LINK = os.path.join(line, 'screenshots/')
        elif line.__contains__('token'):
            line = line.split('=')[1]
            API_KEY = line.strip()

try:
    with open(os.path.join(os.getcwd(), 'workdir', 'screenshots'
                        , 'ScreenshotJsonWithUnitID.json')) as Json_File:
        SCREENSHOTS_WITH_UNIT_ID = json.load(Json_File)
except FileNotFoundError:
    print('Run fetch-unit-id.py first to generate ScreenshotJsonWithUnitID.json file')

try:
    with open(os.path.join(os.getcwd(), 'workdir'
                        , 'screenshots'
                        , 'currentScreenshots.json')) as Json_File:
        CURRENT_SCREENSHOTS = json.load(Json_File)
except FileNotFoundError:
    print("""CurrentScreenshots.json is not found the program will upload
          all the screenshots and repeated screenshots will occure in the units\n
          try to achieve the json file to continue""")
    sys.exit()

try:
    ARGS, VALS = getopt.getopt(ARGUMENT_LIST, SHORT_OPTIONS, LONG_OPTIONS)
except getopt.error as err:
    print(str(err))
    ARGS = []

for argument, currval in ARGS:
    if argument in ["-c", '--config']:
        ConfigFilePath = os.path.expanduser(currval)
        print(ConfigFilePath , currval)

if ARGS != []:
    try:
        fetch_api_token(ConfigFilePath)
    except FileNotFoundError:
        print("Wrong file or file path")
        sys.exit()

try:
    CURRENT_SCREENSHOTS_REQUEST = requests.get(SCREENSHOTSLINK
                                               , headers={"Authorization" :  API_KEY})
except requests.exceptions.HTTPError as error:
    print(error)
    sys.exit()

while True:
    if NEXT_PAGE:
        CURRENT_SCREENSHOTS_RESPONSE = NEXT_PAGE_URL.content
    else:
        CURRENT_SCREENSHOTS_RESPONSE = CURRENT_SCREENSHOTS_REQUEST.content

    SCREENSHOTS = json.loads(CURRENT_SCREENSHOTS_RESPONSE)
    SCREENSHOTS_RESULTS = SCREENSHOTS['results']

    for Screenshot in SCREENSHOTS_RESULTS:
        CURRENT_UPLOADED_SCREENSHOTS[Screenshot['name']] = Screenshot
        screenshoturl = Screenshot['url'].split('/')
        screenshotid = screenshoturl[5]
        IDS_TO_DELETE.append(screenshoturl[5])

    if SCREENSHOTS['next'] is not None:
        NEXT_PAGE = True
        try:
            NEXT_PAGE_URL = requests.get(SCREENSHOTS['next'], headers={"Authorization" :  API_KEY})
        except requests.exceptions.HTTPError as error:
            print(error)
            sys.exit()
    else:
        NEXT_PAGE = False
        break
    if(int((NEXT_PAGE_URL.headers)['X-RateLimit-Remaining'])) < 1000:
        print('Rate limit remaining is below 1000')
    if int((NEXT_PAGE_URL.headers)['X-RateLimit-Remaining']) < 80:
        TIME_WAITING = (CURRENT_SCREENSHOTS_REQUEST.headers)['X-RateLimit-Reset']
        print('program will Wait ', TIME_WAITING, 'seconds not to blocked due to requests limit')
        time.sleep(int((CURRENT_SCREENSHOTS_REQUEST.headers)['X-RateLimit-Reset']))

for id in IDS_TO_DELETE:
    requests.delete(SCREENSHOTSLINK + '/' + id + '/', headers={"Authorization" : API_KEY})
    print('screenshot '+id + ' Deleted')

for screenshotpath in SCREENSHOTS_WITH_UNIT_ID.keys():

    screenshot_signature = get_signature(screenshotpath)
    screenshotname = os.path.basename(screenshotpath)

    if screenshotname in CURRENT_SCREENSHOTS.keys():
        screenshotId = CURRENT_SCREENSHOTS[screenshotname][0]
        # update screenshot if it's changed
        if screenshot_signature != CURRENT_SCREENSHOTS[screenshotname][1]:
            print(screenshot_signature , CURRENT_SCREENSHOTS[screenshotname][1])
            newFileparameter = {'image' : open(os.path.join(
                os.getcwd(), screenshotpath), 'rb')}

            response = requests.post(MAIN_LINK + screenshotId + '/file/'
                                     , files=newFileparameter
                                     , headers={"Authorization" : API_KEY})

        # check on units if there is update on them
        # then add the new units or remove the outdated units

    # Post each screenshot based on the ui files
    # as the component slug changes see choosedatasourcedialog.png
    for ui in SCREENSHOTS_WITH_UNIT_ID[screenshotpath].keys():
        componentslug = (ui.split('/'))[0] + 'messages'
        languagecode = 'en'
        parameters = {
            'name' : screenshotname, 'project_slug' : PROJECT_SLUG,
            'component_slug' : componentslug, 'language_code' : languagecode}

        fileparameter = {'image' : open(os.path.join(os.getcwd(), screenshotpath), 'rb')}
        units_list = []
        try:
            response = requests.post(MAIN_LINK,
                                        files=fileparameter,
                                        data=parameters,
                                        headers={"Authorization" : API_KEY},
                                        timeout=5)
            print('ratelimit remaining :' + (response.headers)['X-RateLimit-Remaining']
                    + ' time remaining to reset '
                    + (CURRENT_SCREENSHOTS_REQUEST.headers)['X-RateLimit-Reset'])

            if int((response.headers)['X-RateLimit-Remaining']) < 80:
                TIME_WAITING = (response.headers)['X-RateLimit-Reset']
                print('screenshtpost program will Wait ', TIME_WAITING
                        , 'seconds not to blocked due to requests limit')
                time.sleep(int((response.headers)['X-RateLimit-Reset']))

            response = response.content
            screenshotresponse = json.loads(response)
            screenshotId = (screenshotresponse['url']).rsplit('/', 1)[0].rsplit('/', 1)[1]
            print('Screenshot number:', screenshotId, ' has been posted')
            units_list = SCREENSHOTS_WITH_UNIT_ID[screenshotpath][ui]
            CURRENT_SCREENSHOTS[screenshotname] = [screenshotId]
            CURRENT_SCREENSHOTS[screenshotname].append(screenshot_signature)
        except ChunkedEncodingError:
            print("chunk")
        """ except Exception as exception:
            EXCEPTIONS_THROWN.append(exception)
            print(exception)
            print(parameters)
            print(response) """


        for unit in units_list:
            url = MAIN_LINK + screenshotId + '/units/'
            try:
                unitid = {'unit_id' : unit[2]}
                responseToAssociation = requests.post(url
                                                      , data=unitid
                                                      , headers={"Authorization" :  API_KEY}
                                                      , timeout=10)

                print('ratelimit remaining :'
                      + (responseToAssociation.headers)['X-RateLimit-Remaining']
                      + 'time remaining to reset limit :'
                      + (responseToAssociation.headers)['X-RateLimit-Reset'])

                if int((responseToAssociation.headers)['X-RateLimit-Remaining']) < 80:
                    TIME_WAITING = (responseToAssociation.headers)['X-RateLimit-Reset']
                    print('assignment program will Wait ', TIME_WAITING,
                          'seconds not to blocked due to requests limit')
                    time.sleep(int((responseToAssociation.headers)['X-RateLimit-Reset']))

                print('Assigned screenshot no.'
                      , screenshotId, ' with with unit of id :'
                      , unit[2])
                CURRENT_SCREENSHOTS[screenshotname].append(unit)
            except IndexError:
                print('The Unit id is missing')
                print(url)
            except Exception as exception:
                print(exception, url)
                NOT_ADDED_UNITS.append(units_list)

with open(os.path.join(os.getcwd(), 'workdir'
                       , 'screenshots'
                       , 'currentScreenshots.json')
          , 'w') as outfile:
    json.dump(CURRENT_SCREENSHOTS, outfile)

# Used it to log the exceptions to be able to look at them after the program finish

with open(os.path.join(os.getcwd()
                       , 'workdir'
                       , 'screenshots'
                       , 'notadded.json')
          , 'w') as outfile:
    json.dump(NOT_ADDED_UNITS, outfile)
