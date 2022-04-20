#!/usr/bin/env python3
# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import configparser
import platform

import sys
import requests


def detect_platform(detected_platform=platform.system()):
    return detected_platform


def main():
    if len(sys.argv) < 4:
        print(sys.argv)
        print("Invalid number of parameters")
        print("Usage: upload-symbols.py symbols.zip config.ini \"long explanation\" [--system | --platform {"
              "Windows|Linux}]")
        sys.exit(1)

    base_url = "https://crashreport.libreoffice.org/"
    upload_url = base_url + "upload/"
    login_url = base_url + "accounts/login/"

    config = configparser.ConfigParser()
    config.read(sys.argv[2])

    user = config["CrashReport"]["User"]
    password = config["CrashReport"]["Password"]

    detected_platform = detect_platform()
    if len(sys.argv) > 5 and sys.argv[4] == "--platform":
        detected_platform = sys.argv[5]

    files = {'symbols': open(sys.argv[1], 'rb')}
    data = {'version': sys.argv[3], 'platform': detected_platform}

    if len(sys.argv) > 4 and sys.argv[4] == "--system":
        data['system'] = True

    session = requests.session()
    session.get(login_url)
    csrftoken = session.cookies['csrftoken']

    login_data = {'username': user, 'password': password,
                  'csrfmiddlewaretoken': csrftoken}
    headers = {"Referer": base_url}
    session.post(login_url, headers=headers, data=login_data)

    data['csrfmiddlewaretoken'] = csrftoken

    session.post(upload_url, headers=headers, files=files, data=data)


if __name__ == "__main__":
    main()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
