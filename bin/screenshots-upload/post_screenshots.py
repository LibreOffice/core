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
import pickle
import os
import json
import sys
import subprocess
import requests

from fetch_unit_id import(next_page_request, rate_limit_exceeded
                          , check_and_get_params,SCREENSHOTS_DIR_PATH)
from requests.models import ChunkedEncodingError
SCREENSHOTSLINK = 'https://weblate-gsoc.documentfoundation.org/api/screenshots/'
ARGUMENT_LIST = sys.argv[1:]
SHORT_OPTIONS = "c:"
LONG_OPTIONS = ["config=","force"]
API_KEY = ''
PROJECT_SLUG = 'libo_ui-master'
MAIN_LINK = 'https://translations.documentfoundation.org/api/'
AUTH_HEADER = {"Authorization":  API_KEY}
ids_to_delete = []
current_uploaded_screenshots = {}
current_screenshots = {}
contain_next_page = False
not_added_units = []
exceptions_thrown = []
next_page_response = None
anonymous = True

def assign_unit(screenshotId, unit, AUTH_HEADER
                , current_screenshots, screenshotpath):
    global not_added_units

    url = MAIN_LINK + screenshotId + '/units/'
    unitid = {'unit_id' : unit[2]}
    responseToAssociation = requests.post(url
                                        , data=unitid
                                        , headers=AUTH_HEADER
                                        , timeout=5)
    responseToAssociation.raise_for_status()
    limit_ack(responseToAssociation.headers)
    if int((responseToAssociation.headers)['X-RateLimit-Remaining']) < 80:
        rate_limit_exceeded(
        (responseToAssociation.headers)['X-RateLimit-Reset'])

    print('Assigned screenshot no.'
        , screenshotId, ' with with unit of id :'
        , unitid)
    current_screenshots[screenshotpath].append(unit)

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
    with open(os.path.join(SCREENSHOTS_DIR_PATH
                           , 'ScreenshotJsonWithUnitID.json')) as Json_File:
        screenshot_to_strings = json.load(Json_File)
except FileNotFoundError as e:
    print(e)
    print('Run fetch-unit-id.py first to generate ScreenshotJsonWithUnitID.json file')
    sys.exit()


with open(os.path.join(SCREENSHOTS_DIR_PATH
                       , 'currentScreenshots.json')) as Json_File:
    current_screenshots = json.load(Json_File)

if(not ARGUMENT_LIST.__len__() == 0):
    returned_list = check_and_get_params(ARGUMENT_LIST)
    if not returned_list.__len__() == 3:
        MAIN_LINK = os.path.join(returned_list[0][0], 'screenshots/')
        API_KEY = returned_list[0][1]
        anonymous = returned_list[1]
        AUTH_HEADER = {"Authorization":  API_KEY}
        if (returned_list[0]).__len__() == 3:
                PROJECT_SLUG = returned_list[0][2]

current_screenshots_request = requests.get(SCREENSHOTSLINK
                                           , headers=AUTH_HEADER)
current_screenshots_request.raise_for_status()

for screenshotpath in screenshot_to_strings.keys():

    screenshot_signature = get_signature(screenshotpath)
    screenshotname = os.path.basename(screenshotpath)

    if screenshotpath in current_screenshots.keys():
        screenshotId = current_screenshots[screenshotpath][0]
        # update screenshot if it's changed
        if screenshot_signature != current_screenshots[screenshotpath][1]:
            print('screenshot with id {} has been changed so it will be replaced'.format(screenshotId))
            newFileparameter = {'image' : open(os.path.join(
                os.getcwd(), screenshotpath), 'rb')}

            response = requests.post(MAIN_LINK + screenshotId + '/file/'
                                    , files=newFileparameter
                                    , headers=AUTH_HEADER)
            response.raise_for_status()
            limit_ack(response.headers)
            current_screenshots[screenshotpath][1] = screenshot_signature
            print('screenshot replaced successfully')
        else:
            print('screenshot with id {} has not been changed'.format(screenshotId))
        # check on units if there is update on them
        # then add the new units or remove the outdated units
        for ui in screenshot_to_strings[screenshotpath].keys():
            for unit in screenshot_to_strings[screenshotpath][ui]:
                found_in_currentunits = False
                for unit_proprties in current_screenshots[screenshotpath]:
                    if (unit_proprties.__len__() == 3
                        and (unit_proprties[0] == unit[0]
                        and unit_proprties[1] == unit[1]
                        and unit_proprties[1] == unit[1])):
                            found_in_currentunits = True
                            break
                if not found_in_currentunits:
                    print(unit)
                    assign_unit(screenshotId, unit, AUTH_HEADER
                                , current_screenshots, screenshotpath)

    else:
        # If not found on weblate/currentscreenshots
        # post each screenshot based on the ui files
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
            response.raise_for_status()
            limit_ack(response.headers)

            if int((response.headers)['X-RateLimit-Remaining']) < 80:
                rate_limit_exceeded((response.headers)['X-RateLimit-Reset'])

            response = response.content
            screenshotresponse = json.loads(response)
            screenshotId = (screenshotresponse['url']).rsplit('/', 1)[0].rsplit('/', 1)[1]
            print('Screenshot number:', screenshotId, ' has been posted')
            units_list = screenshot_to_strings[screenshotpath][ui]
            current_screenshots[screenshotpath] = [screenshotId]
            current_screenshots[screenshotpath].append(screenshot_signature)

            for unit in units_list:
                assign_unit(screenshotId, unit, AUTH_HEADER
                            , current_screenshots, screenshotpath)

with open(os.path.join(SCREENSHOTS_DIR_PATH
                       , 'currentScreenshots.json')
          , 'w') as outfile:
    json.dump(current_screenshots, outfile)

# Used it to log the exceptions to be able to look at them after the program finish
for x in not_added_units:
    print(x)

