#!/usr/bin/env python3
#
# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
""" The script is used to delete all the screenshots
    and post the updated screenshots to Weblate and assign their unit ids ."""
import getopt
import os
import json
import sys
import time
import subprocess
import requests
from fetch_unit_id import(MAIN_LINK
                          , next_page_request, rate_limit_exceeded
                          , check_and_get_params)
from requests.models import ChunkedEncodingError

SCREENSHOTSLINK = 'https://weblate-gsoc.documentfoundation.org/api/screenshots/'
ARGUMENT_LIST = sys.argv[1:]
SHORT_OPTIONS = "c:"
LONG_OPTIONS = ["config=","force="]
API_KEY = ''
PROJECT_SLUG = 'libo_ui-master'
ids_to_delete = []
current_uploaded_screenshots = {}
current_screenshots = {}
contain_next_page = False
not_added_units = []
exceptions_thrown = []
next_page_url = ''
anonymous = False

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

def limit_ack(headers):
    print('ratelimit remaining :' + (headers)['X-RateLimit-Remaining']
            + ' time remaining to reset '
            + (headers)['X-RateLimit-Reset'])
try:
    with open(os.path.join(os.getcwd(), 'workdir', 'screenshots'
                        , 'ScreenshotJsonWithUnitID.json')) as Json_File:
        screenshot_to_strings = json.load(Json_File)
except FileNotFoundError:
    print('Run fetch-unit-id.py first to generate ScreenshotJsonWithUnitID.json file')
    sys.exit()

try:
    with open(os.path.join(os.getcwd(), 'workdir'
                        , 'screenshots'
                        , 'currentScreenshots.json')) as Json_File:
        current_screenshots = json.load(Json_File)
except FileNotFoundError:
    print(""" screenshots/currentScreenshots.json is not found the program will upload
          all the screenshots and repeated screenshots will occure in the units\n
          try to achieve the json file to continue""")
    sys.exit()

check_and_get_params(anonymous)

current_screenshots_request = requests.get(SCREENSHOTSLINK
                                           , headers={"Authorization" :  API_KEY})
current_screenshots_request.raise_for_status()


while True:
    if contain_next_page:
        current_screenshots_response = next_page_url.content
    else:
        current_screenshots_response = current_screenshots_request.content

    screenshots = json.loads(current_screenshots_response)
    screenshots_results = screenshots['results']

    for Screenshot in screenshots_results:
        current_uploaded_screenshots[Screenshot['name']] = Screenshot
        screenshoturl = Screenshot['url'].split('/')
        screenshotid = screenshoturl[5]
        ids_to_delete.append(screenshoturl[5])

    if screenshots['next'] is not None:
        contain_next_page = True
        next_page_request()
    else:
        contain_next_page = False
        break
    if(int((next_page_url.headers)['X-RateLimit-Remaining'])) < 1000:
        print('Rate limit remaining is below 1000')
    if int((next_page_url.headers)['X-RateLimit-Remaining']) < 80:
        rate_limit_exceeded((next_page_url.headers)['X-RateLimit-Reset'])

for id in ids_to_delete:
    requests.delete(SCREENSHOTSLINK + '/' + id + '/', headers={"Authorization" : API_KEY})
    print('screenshot '+id + ' Deleted')

for screenshotpath in screenshot_to_strings.keys():

    screenshot_signature = get_signature(screenshotpath)
    screenshotname = os.path.basename(screenshotpath)

    if screenshotname in current_screenshots.keys():
        screenshotId = current_screenshots[screenshotname][0]
        # update screenshot if it's changed
        if screenshot_signature != current_screenshots[screenshotname][1]:
            print(screenshot_signature , current_screenshots[screenshotname][1])
            newFileparameter = {'image' : open(os.path.join(
                os.getcwd(), screenshotpath), 'rb')}

            response = requests.post(MAIN_LINK + screenshotId + '/file/'
                                     , files=newFileparameter
                                     , headers={"Authorization" : API_KEY})

        # check on units if there is update on them
        # then add the new units or remove the outdated units

    # Post each screenshot based on the ui files
    # as the component slug changes see choosedatasourcedialog.png
    for ui in screenshot_to_strings[screenshotpath].keys():
        componentslug = (ui.split('/'))[0] + 'messages'
        languagecode = 'en'
        parameters = {
            'name' : screenshotname, 'project_slug' : PROJECT_SLUG,
            'component_slug' : componentslug, 'language_code' : languagecode}

        fileparameter = {'image' : open(os.path.join(os.getcwd(), screenshotpath), 'rb')}
        units_list = []

        response = requests.post(MAIN_LINK,
                                    files=fileparameter,
                                    data=parameters,
                                    headers={"Authorization" : API_KEY},
                                    timeout=5)
        limit_ack(response.headers)

        if int((response.headers)['X-RateLimit-Remaining']) < 80:
            rate_limit_exceeded((response.headers)['X-RateLimit-Reset'])

        response = response.content
        screenshotresponse = json.loads(response)
        screenshotId = (screenshotresponse['url']).rsplit('/', 1)[0].rsplit('/', 1)[1]
        print('Screenshot number:', screenshotId, ' has been posted')
        units_list = screenshot_to_strings[screenshotpath][ui]
        current_screenshots[screenshotname] = [screenshotId]
        current_screenshots[screenshotname].append(screenshot_signature)

        for unit in units_list:
            url = MAIN_LINK + screenshotId + '/units/'
            try:
                unitid = {'unit_id' : unit[2]}
                responseToAssociation = requests.post(url
                                                      , data=unitid
                                                      , headers={"Authorization" :  API_KEY}
                                                      , timeout=5)

                limit_ack(responseToAssociation.headers)
                if int((responseToAssociation.headers)['X-RateLimit-Remaining']) < 80:
                    rate_limit_exceeded((responseToAssociation.headers)['X-RateLimit-Reset'])

                print('Assigned screenshot no.'
                      , screenshotId, ' with with unit of id :'
                      , unit[2])
                current_screenshots[screenshotname].append(unit)
            except IndexError:
                print('The Unit id is missing' , unit)
                print(url)
                not_added_units.append((unit , url))
            except Exception as exception:
                print(exception, url)
                not_added_units.append((unit,url))
                sys.exit()

with open(os.path.join(os.getcwd(), 'workdir'
                       , 'screenshots'
                       , 'currentScreenshots.json')
          , 'w') as outfile:
    json.dump(current_screenshots, outfile)

# Used it to log the exceptions to be able to look at them after the program finish

with open(os.path.join(os.getcwd()
                       , 'workdir'
                       , 'screenshots'
                       , 'notadded.json')
          , 'w') as outfile:
    json.dump(not_added_units, outfile)
