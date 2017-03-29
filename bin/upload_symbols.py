#!/usr/bin/env python3
# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import requests, sys
import platform, configparser

def detect_platform():
    return platform.system()

def main():
    if len(sys.argv) < 4:
        print(sys.argv)
        print("Invalid number of parameters")
        print("Usage: upload-symbols.py symbols.zip config.ini \"long explanation\" [--system]")
        sys.exit(1)

    upload_url = "http://crashreport.collaboraoffice.com/upload/"
    login_url = "http://crashreport.collaboraoffice.com/accounts/login/"

    config = configparser.ConfigParser()
    config.read(sys.argv[2])

    user = config["CrashReport"]["User"]
    password = config["CrashReport"]["Password"]

    platform = detect_platform()
    files = {'symbols': open(sys.argv[1], 'rb')}
    data = {'version': sys.argv[3], 'platform': platform}

    if len(sys.argv) > 4 and sys.argv[4] == "--system":
        data['system'] = True

    session = requests.session()
    session.get(login_url)
    csrftoken = session.cookies['csrftoken']

    login_data = { 'username': user,'password': password,
            'csrfmiddlewaretoken': csrftoken }
    r1 = session.post(login_url,data=login_data)

    data['csrfmiddlewaretoken'] = csrftoken

    r = session.post(upload_url, files = files, data = data)

if __name__ == "__main__":
    main()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
