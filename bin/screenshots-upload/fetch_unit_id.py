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
from requests import api
SHORT_OPTIONS = "c:"
LONG_OPTIONS = ["config=" , "force", "branch="]
SCREENSHOTS_DIR_PATH = os.path.join(os.getcwd(), 'workdir', 'screenshots')

def rate_limit_exceeded(time_to_wait):
    """Wait the RateLimit reset"""
    print("""program will wait {} seconds
not to blocked due to requests limit""".format(time_to_wait))
    time.sleep(int(time_to_wait))


def fetch_api_token(config_file_path):
    """Extract the Api_key and the link from the config file"""
    config_file_path = os.path.expanduser(config_file_path)
    configfile = open(config_file_path, 'r')


    for line in configfile:
        if line.__contains__('URL'):
            line = line.split('=')[1]
            line = line.strip()
            MAIN_LINK = line
        elif line.__contains__('token'):
            line = line.split('=')[1]
            API_KEY = line.strip()
    return [MAIN_LINK, API_KEY]


def next_page_request(url , AUTH_HEADER):
    next_page_response = requests.get(url
                                 , headers=AUTH_HEADER
                                 , timeout=5)
    next_page_response.raise_for_status()
    return next_page_response


def check_and_get_params(ARGUMENT_LIST):
    if(ARGUMENT_LIST[0] not in ["-c", '--config' , '--force', '--branch']):
        print('Wrong options are passed the valid options are -c , --config or --force')
        sys.exit()

    args, vals = getopt.getopt(ARGUMENT_LIST, SHORT_OPTIONS, LONG_OPTIONS)
    returned_values = []
    for argument, currval in args:
        if argument in ["-c" , '--config']:
            ConfigFilePath = os.path.expanduser(currval)
            api_token = fetch_api_token(ConfigFilePath)
            returned_values.append(api_token)
            returned_values.append(False)
            #return [fetch_api_token(ConfigFilePath), False]
        elif argument in ['--force']:
            returned_values.append('','',True)
            #return ['', '', True]
        elif argument in ['--branch']:
            returned_values[0].append(currval)

    return returned_values

def main():
    argument_list = sys.argv[1:]
    API_KEY = ''
    MAIN_LINK = 'https://translations.documentfoundation.org/api/translations/'
    PROJECT_SLUG = 'libo_ui-master/'
    LANGUAGE = 'en/'
    AUTH_HEADER = {"Authorization": ''}
    screenshot_to_strings = {}
    keyid_to_unitid = {}
    screenshot_count = 0
    anonymous = True
    forced = False

    if(not argument_list.__len__() == 0):
        returned_list = (check_and_get_params(argument_list))
        # That means that the returned values are 3 and anonymous is true
        if (returned_list).__len__() == 3:
            forced = True
        else:
            MAIN_LINK = os.path.join(returned_list[0][0], 'translations/')
            API_KEY = returned_list[0][1]
            AUTH_HEADER = {"Authorization":  API_KEY}
            anonymous = False
            if (returned_list[0]).__len__() == 3:
                PROJECT_SLUG = returned_list[0][2]
    MAIN_LINK = os.path.join(MAIN_LINK , PROJECT_SLUG) + '/'

    # Convert Map < screenshotPath -> (uiPath, context)Pair > in json to dict
    try:
        with open(os.path.join(SCREENSHOTS_DIR_PATH , 'ScreenshotJson.json')) as Json_File:
            screenshot_to_strings = json.load(Json_File)
    except FileNotFoundError as e:
        print(e)
        print(""" you just ran {}
Try to run \'bin/screenshots-upload/mapping.py\'
script first to generate screenshotjson file
""".format(SCREENSHOTS_DIR_PATH + '/ScreenshotJson.json'))
        sys.exit()


    for screenshotLocation in screenshot_to_strings.keys():
        screenshot_count += 1
        for UIPath in screenshot_to_strings[screenshotLocation]:
            componentSlug = UIPath.split('/')[0] + ('messages/')
            Location = 'location:' + UIPath
            query = {'q': Location}

            request = requests.get(MAIN_LINK
                                    + componentSlug
                                    + LANGUAGE
                                    + 'units/'
                                    , params=query
                                    , headers=AUTH_HEADER
                                    , timeout=5)
            request.raise_for_status()

            contain_next_page = False

            if screenshot_count == 1 and request.headers['X-RateLimit-Limit'] == '100':
                if(not anonymous):
                    print("You've typed wrong authentication token")
                elif(not forced):
                    print("""The program will abort as you don't have authentication token
you'll be blocked many times due to api requests limits
if you want to continue run the script with --force switch""")

                sys.exit()

            elif int((request.headers)['X-RateLimit-Remaining']) < 1000:
                print('Rate limit remaining is below 1000')
                if int((request.headers)['X-RateLimit-Remaining']) < 80:
                    rate_limit_exceeded(int((request.headers)['X-RateLimit-Reset']))

            while True:

                if contain_next_page:
                    units_response = next_page_response.content
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
                    next_page_response = next_page_request(units['next'],AUTH_HEADER)
                else:
                    break
                if int((next_page_response.headers)['X-RateLimit-Remaining']) < 1000:
                    print('Rate limit remaining is below 1000')
                    if int((next_page_response.headers)['X-RateLimit-Remaining']) < 80:
                        rate_limit_exceeded(int((next_page_response.headers)['X-RateLimit-Reset']))

        print(len(screenshot_to_strings.keys()) - screenshot_count, "screenshots remaining")

    # Save the mapping of keyid of context to unit idd .
    txt_file_path = os.path.join(SCREENSHOTS_DIR_PATH , 'KeyIdtoUnitID.json')
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


    txt_file_path = os.path.join(SCREENSHOTS_DIR_PATH, 'ScreenshotJsonWithUnitID.json')
    with open(txt_file_path, 'w') as outfile:
        json.dump(screenshot_to_strings, outfile)

if __name__ == "__main__":
    main()
