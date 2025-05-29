#!/usr/bin/env python3

# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# Use this script to compare the crashreport stats of two different versions of LibreOffice.
# Usage sample: ./compare-crashreport-stats.py --old 7.2.0.4 --new 7.2.1.2

import requests
from bs4 import BeautifulSoup
import argparse

def parse_url(version : str, session = requests):
    crashReports = {}
    url = "https://crashreport.libreoffice.org/stats/version/" + version + "?limit=1000&days=30"
    html_text = session.get(url).text
    soup = BeautifulSoup(html_text, 'html.parser')

    table = soup.find("table", {"id": "data-table"}).tbody
    for tr in table.find_all("tr"):
        td_list = tr.find_all("td")
        crashName = td_list[0].a.text.strip()
        crashNumber = int(td_list[1].text.strip())
        crashReports[crashName] = crashNumber

    return crashReports

if __name__ == '__main__':
    parser = argparse.ArgumentParser()

    parser.add_argument('--old', action='store', dest="old", required=True)
    parser.add_argument('--new', action="store", dest="new", required=True)

    results = parser.parse_args()

    session = requests.Session()
    session.headers.update({'Referer': 'https://crashreport.libreoffice.org'})
    oldVersion = parse_url(results.old, session=session)
    newVersion = parse_url(results.new, session=session)

    print(str(len(oldVersion)) + " crash reports in version " + results.old)
    print(str(len(newVersion)) + " crash reports in version " + results.new)
    print()

    print("===== Fixed Reports =====")
    fixedReports = set(oldVersion.keys()) - set(newVersion.keys())
    for k, v in sorted(oldVersion.items(), key=lambda item: item[1], reverse=True):
        # Ignore rare reports
        if v >= 20 and k in fixedReports:
            print(str(v) + " " + k)

    print()
    print("===== Newly introduced Reports =====")
    newReports = set(newVersion.keys()) - set(oldVersion.keys())
    for k, v in sorted(newVersion.items(), key=lambda item: item[1], reverse=True):
        # Ignore rare reports
        if v >= 20 and k in newReports:
            print(str(v) + " " + k)
    print()
