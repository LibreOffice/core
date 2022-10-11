#!/usr/bin/env python3
# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import configparser
import platform
import argparse
import requests


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('symbols_zip_file')
    parser.add_argument('credentials_file')
    parser.add_argument('version')
    group = parser.add_mutually_exclusive_group()
    group.add_argument('--system', action='store_true')
    group.add_argument('--platform', choices=('Windows', 'Linux'))
    args = parser.parse_args()

    base_url = "https://crashreport.libreoffice.org/"
    upload_url = base_url + "upload/"
    login_url = base_url + "accounts/login/"

    config = configparser.ConfigParser()
    config.read(args.credentials_file)

    user = config["CrashReport"]["User"]
    password = config["CrashReport"]["Password"]

    files = {'symbols': open(args.symbols_zip_file, 'rb')}
    data = {'version': args.version,
            'system': args.system,
            'platform': platform.system() if args.platform is None else args.platform}

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
