#!/usr/bin/env python3
#
# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
""" The script iterate on every screenshot and map them to their UI files
    then maps every UI path to context and keyid"""

import os
import json

SCREENSHOTS_DIR_PATH = os.path.join(os.getcwd(), 'workdir', 'screenshots')
POT_DIR_PATH = os.path.join(os.getcwd(), 'workdir', 'pot')
TXT_FILE_PATH = os.path.join(SCREENSHOTS_DIR_PATH, 'ScreenshotJson.json')
screenshot_to_strings = {}

def search_in_text(file_path, target_ui, screenshot_path):
    """Open POT file and search for the ui_path
    and put context and key id with uipath
    note :sometimes context is two lines"""
    twolinescontext = False
    next_line_is_context = False
    key = ''
    ui_path = ''
    uipath_with_context = {}

    with open(file_path, 'r', encoding='utf-8') as file:
        for line in file:
            if line.__contains__('#.'):
                keyid = line.split(' ', 1)
                if twolinescontext:
                    key = key + '\n'+ (keyid[1])[:-1]
                    twolinescontext = False
                else:
                    key = (keyid[1])[:-1]
                    twolinescontext = True
            elif next_line_is_context:
                twolinescontext = False
                context_second_line = line.split("\"")

                context = context_second_line[1][0:len(context_second_line[1])] #Remove quotes

                screenshot_to_strings[screenshot_path][ui_path].append(
                    (context, key))
                print(target_ui + ' mapped to (context,key) pair : '
                + context + ' ' + key)
                next_line_is_context = False

            elif line.find('/' + target_ui.lower()) != -1:
                twolinescontext = False
                ui_path = line[3:-1]
                temp = ui_path.split(':') #Ignore line number
                ui_path = temp[0]

                if screenshot_path not in screenshot_to_strings.keys():
                    uipath_with_context[ui_path] = []
                    screenshot_to_strings[screenshot_path] = uipath_with_context
                else:
                    if ui_path not in screenshot_to_strings[screenshot_path]:
                        screenshot_to_strings[screenshot_path][(ui_path)] = []

                next_line_is_context = True
            else:
                twolinescontext = False

def find_in_pot(target_ui, screenshot_path):
    """Iterate over all the POT files
        then calls search_in_text on the POT file"""
    for dir_path, dir_names, pot_files in os.walk(POT_DIR_PATH): #traverse all pot files
        if len(pot_files) != 0:
            # Search in the text of the pot file with the target UI
            pot_file_path = os.path.join(dir_path, pot_files[0])
            search_in_text(pot_file_path, target_ui, screenshot_path)

# Map contains:  screenshot_path -> {ui_path->{context, KeyId} }

# Traverse all the directories and files in screenshot_path

for dirpath, dirnames, screenshots_files in os.walk(SCREENSHOTS_DIR_PATH):

    # If valid file
    if len(screenshots_files) != 0:
        ui_filename = os.path.basename(os.path.normpath(dirpath)) + '.ui'
        # Screenshot location -
        # is just dirpath + screenshot name
        screenshot_image_path = os.path.join(dirpath, screenshots_files[0])

        # Pass ui_filename to be searched in the pot file
        screenshot_image_path = os.path.relpath(screenshot_image_path)
        #make it relative

        find_in_pot(ui_filename, screenshot_image_path)


# Save map to json format in txt file
with open(TXT_FILE_PATH, 'w')as outfile:
    json.dump(screenshot_to_strings, outfile)
