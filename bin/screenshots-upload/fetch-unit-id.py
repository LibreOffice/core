""" The script GET unit id from the Weblate API and assign them to the screenshots
    Note: Mapping.py should be run first
"""
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
import time
import sys
import getopt
import requests

TXT_FILE_PATH = os.path.join(os.getcwd(), 'workdir', 'screenshots', 'ScreenshotJson.json')
ARGUMENT_LIST = sys.argv[1:]
SHORT_OPTIONS = "c:"
LONG_OPTIONS = ["config="]
screenshot_to_strings = {}
KEY_ID_TO_UNIT_ID = {}
SCREENSHOT_COUNT = 0
API_KEY = ''
MAIN_LINK = ''
PROJECT_SLUG = 'libo_ui-master/'
LANGUAGE = 'en/'
def rate_limit_exceeded(time_to_wait):
    """Wait the RateLimit reset"""
    print('program will Wait ', time_to_wait, 'seconds not to blocked due to requests limit')
    time.sleep(time_to_wait)

def fetch_api_token(config_file_path):
    """Extract the Api_key and the link from the config file"""
    global API_KEY, MAIN_LINK
    config_file_path = os.path.expanduser(config_file_path)
    configfile = open(config_file_path, 'r')
    for line in configfile:
        if line.__contains__('URL'):
            line = line.split('=')[1]
            line = line.strip()
            MAIN_LINK = os.path.join(line, 'translations/')
        elif line.__contains__('token'):
            line = line.split('=')[1]
            API_KEY = line.strip()

try:
    ARGS, VALS = getopt.getopt(ARGUMENT_LIST, SHORT_OPTIONS, LONG_OPTIONS)
except getopt.error as err:
    print(str(err))
    ARGS = []

for argument, currval in ARGS:
    if(argument in ["-c", '--config']):
        ConfigFilePath = os.path.expanduser(currval)


if ARGS != []:
    try:
        fetch_api_token(ConfigFilePath)
    except FileNotFoundError:
        print("Wrong file or file path")
        sys.exit()

#else:
#    print(""" YOU ARE IN ANONYMOUS MODE , YOU DIDN\'T ADD ANY CONFIG FILE
#     SO YOU WILLBE BLOCKED FOR HOURS TO RUN THE PROGRAM AND IT MAY FAIL
#     YOU CAN ADD CONFIG FILE PATH BY TYPING -c OR --config AND THEN YOUR FILEPATH""" )

# Convert Map < screenshotPath -> (uiPath, context)Pair > in json to dict
try:
    with open(os.path.join(os.getcwd(), 'workdir',
                           'screenshots',
                           'ScreenshotJson.json')) as Json_File:
        screenshot_to_strings = json.load(Json_File)
except FileNotFoundError:
    print('Try to run \'Mapping.py\' script first to generate screenshotjson file')
    sys.exit()


for screenshotLocation in screenshot_to_strings.keys():
    SCREENSHOT_COUNT += 1
    for UIPath in screenshot_to_strings[screenshotLocation]:
        componentSlug = UIPath.split('/')[0] + ('messages/')
        Location = 'location:' + UIPath
        query = {'q': Location}
        try:
            request = requests.get(MAIN_LINK + PROJECT_SLUG
                                   + componentSlug
                                   + LANGUAGE
                                   + 'units/'
                                   , params=query
                                   , headers={"Authorization":  API_KEY}
                                   , timeout=5)
            request.raise_for_status()
            nextPageUrl = ''
            ContainNextPage = False
        except requests.exceptions.HTTPError as error:
            print(error)
            sys.exit()

        if SCREENSHOT_COUNT == 1 and request.headers['X-RateLimit-Limit'] == '100':
            print(""" YOU ARE IN ANONYMOUS MODE , YOU DIDN\'T ADD ANY CONFIG FILE
SO YOU WILLBE BLOCKED FOR HOURS TO RUN THE PROGRAM AND IT MAY FAIL
YOU CAN ADD CONFIG FILE PATH NOW IF YOU DON\'T have ONE THEN TYPE -1""")

            ConfigFilePath = input()
            if ConfigFilePath != '-1':
                fetch_api_token(ConfigFilePath)
            else:
                sys.exit()

        elif int((request.headers)['X-RateLimit-Remaining']) < 1000:
            print('Rate limit remaining is below 1000')
        if int((request.headers)['X-RateLimit-Remaining']) < 80:
            rate_limit_exceeded(int((request.headers)['X-RateLimit-Reset']))

        while True:

            if ContainNextPage:
                response = nextPageUrl.content
            else:
                response = request.content

            # Save response
            units = json.loads(response)
            UnitsResults = units['results']

            # Iterate over the response and screenshot_to_strings every keyid of specific context
            # to it's Unit id
            for unit in UnitsResults:
                KEY_ID_TO_UNIT_ID[unit['note']] = unit['id']

            #if there is next page put the api key and request the next page
            if units['next'] is not None:
                try:
                    ContainNextPage = True
                    nextPageUrl = requests.get(units['next']
                                               , headers={"Authorization":  API_KEY}
                                               , timeout=5)
                except requests.exceptions.HTTPError as error:
                    print(error)
                    sys.exit()
            else:
                break
            if int((request.headers)['X-RateLimit-Remaining']) < 1000:
                print('Rate limit remaining is below 1000')
            if int((nextPageUrl.headers)['X-RateLimit-Remaining']) < 80:
                rate_limit_exceeded(int((request.headers)['X-RateLimit-Reset']))

    print(len(screenshot_to_strings.keys()) - SCREENSHOT_COUNT, "screenshot remaining")

# Save the mapping of keyid of context to unit idd .
TXT_FILE_PATH = os.path.join(os.getcwd(), 'workdir', 'screenshots', 'KeyIdtoUnitID.json')
with open(TXT_FILE_PATH, 'w') as outfile:
    json.dump(KEY_ID_TO_UNIT_ID, outfile)


# Put the unit id of context in the screenshot_to_strings and save them .

for screenshotLocation in screenshot_to_strings.keys():

    for ui in screenshot_to_strings[screenshotLocation]:
        for l in screenshot_to_strings[screenshotLocation][ui]:
            try:
                l.append(KEY_ID_TO_UNIT_ID[l[1]])
            except KeyError:
                print('unit: ' + l[1] + ' is not found')


TXT_FILE_PATH = os.path.join(os.getcwd(), 'workdir', 'screenshots', 'ScreenshotJsonWithUnitID.json')
with open(TXT_FILE_PATH, 'w') as outfile:
    json.dump(screenshot_to_strings, outfile)
