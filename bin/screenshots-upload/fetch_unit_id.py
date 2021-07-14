#!/usr/bin/env python3
#
# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
""" The script gets the  unit IDs using the Weblate API and assign them to the screenshots
    Note: Mapping.py run should be first
"""
import os
import json
import time
import sys
import getopt
import requests

screenshots_dir_path = os.path.join(os.getcwd(), 'workdir', 'screenshots')
ARGUMENT_LIST = sys.argv[1:]
SHORT_OPTIONS = "c:"
LONG_OPTIONS = ["config=" , "force="]
API_KEY = ''
MAIN_LINK = ''
PROJECT_SLUG = 'libo_ui-master/'
LANGUAGE = 'en/'
screenshot_to_strings = {}
keyid_to_unitid = {}
screenshot_count = 0
anonymous = False


def rate_limit_exceeded(time_to_wait):
    """Wait the RateLimit reset"""
    print("""program will wait {} seconds
             not to blocked due to requests limit""".format(time_to_wait))
    time.sleep(time_to_wait)


def fetch_api_token(config_file_path):
    """Extract the Api_key and the link from the config file"""
    global API_KEY, MAIN_LINK
    config_file_path = os.path.expanduser(config_file_path)
    try:
        configfile = open(config_file_path, 'r')
    except FileNotFoundError:
        print("No config file found at path {}".format(config_file_path))
        sys.exit()

    for line in configfile:
        if line.__contains__('URL'):
            line = line.split('=')[1]
            line = line.strip()
            MAIN_LINK = os.path.join(line, 'translations/')
        elif line.__contains__('token'):
            line = line.split('=')[1]
            API_KEY = line.strip()


def next_page_request():
    next_page_url = requests.get(units['next']
                                , headers={"Authorization":  API_KEY}
                                , timeout=5)
    next_page_url.raise_for_status()


def check_and_get_params(anonymous):
    if(ARGUMENT_LIST[0] not in ["-c", '--config' , '--force']):
        print('Wrong options are passed the valid options are -c , --config or --force')
        sys.exit()

    args, vals = getopt.getopt(ARGUMENT_LIST, SHORT_OPTIONS, LONG_OPTIONS)
    for argument, currval in args:
        if argument in ["-c" , '--config']:
            ConfigFilePath = os.path.expanduser(currval)
            fetch_api_token(ConfigFilePath)
        elif argument in ['--force']:
            print(""" You are in anonymouos mode , you didn\'t add any config file
    so you'll be blocked many times due to api requests limits""")
            anonymous = True



check_and_get_params(anonymous)
# Convert Map < screenshotPath -> (uiPath, context)Pair > in json to dict
try:
    with open(os.path.join(screenshots_dir_path , 'ScreenshotJson.json')) as Json_File:
        screenshot_to_strings = json.load(Json_File)
except FileNotFoundError:
    print(""" you just ran {}
          \nTry to run \'bin/screenshots-upload/mapping.py\'
          script first to generate screenshotjson file
          """.format(screenshots_dir_path + '/ScreenshotJson.json'))
    sys.exit()


for screenshotLocation in screenshot_to_strings.keys():
    screenshot_count += 1
    for UIPath in screenshot_to_strings[screenshotLocation]:
        componentSlug = UIPath.split('/')[0] + ('messages/')
        Location = 'location:' + UIPath
        query = {'q': Location}

        request = requests.get(MAIN_LINK + PROJECT_SLUG
                                + componentSlug
                                + LANGUAGE
                                + 'units/'
                                , params=query
                                , headers={"Authorization":  API_KEY}
                                , timeout=5)
        request.raise_for_status()
        next_page_url = ''
        contain_next_page = False

        if screenshot_count == 1 and request.headers['X-RateLimit-Limit'] == '100' and not anonymous:
            print(""" You've typed wrong authentication token""")
            sys.exit()

        elif int((request.headers)['X-RateLimit-Remaining']) < 1000:
            print('Rate limit remaining is below 1000')
        if int((request.headers)['X-RateLimit-Remaining']) < 80:
            rate_limit_exceeded(int((request.headers)['X-RateLimit-Reset']))

        while True:

            if contain_next_page:
                units_response = next_page_url.content
            else:
                units_response = request.content

            # Save units_response
            units = json.loads(units_response)
            UnitsResults = units['results']

            # Iterate over the units_response and screenshot_to_strings every keyid of specific context
            # to it's Unit id
            for unit in UnitsResults:
                keyid_to_unitid[unit['note']] = unit['id']

            #if there is next page put the api key and request the next page
            if units['next'] is not None:
                contain_next_page = True
                next_page_url = requests.get(units['next']
                                            , headers={"Authorization":  API_KEY}
                                            , timeout=5)
                next_page_url.raise_for_status()

            else:
                break
            if int((request.headers)['X-RateLimit-Remaining']) < 1000:
                print('Rate limit remaining is below 1000')
            if int((next_page_url.headers)['X-RateLimit-Remaining']) < 80:
                rate_limit_exceeded(int((request.headers)['X-RateLimit-Reset']))

    print(len(screenshot_to_strings.keys()) - screenshot_count, "screenshots remaining")

# Save the mapping of keyid of context to unit idd .
txt_file_path = os.path.join(screenshots_dir_path , 'KeyIdtoUnitID.json')
with open(txt_file_path, 'w') as outfile:
    json.dump(keyid_to_unitid, outfile)


# Put the unit id of context in the screenshot_to_strings and save them .

for screenshotLocation in screenshot_to_strings.keys():

    for ui in screenshot_to_strings[screenshotLocation]:
        for l in screenshot_to_strings[screenshotLocation][ui]:
            try:
                l.append(keyid_to_unitid[l[1]])
            except KeyError:
                print('unit: ' + l[1] + ' is not found')


txt_file_path = os.path.join(screenshots_dir_path, 'ScreenshotJsonWithUnitID.json')
with open(txt_file_path, 'w') as outfile:
    json.dump(screenshot_to_strings, outfile)
