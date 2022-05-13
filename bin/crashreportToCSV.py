#!/usr/bin/env python3

# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# Use this script to retrieve information from https://crashreport.libreoffice.org
# about a specific version of LibreOffice
# Usage sample: ./crashreportToCSV.py 7.2.0.4

import requests
from bs4 import BeautifulSoup
import sys
import os
import math
from datetime import datetime

def convert_str_to_date(value):
    value = value.replace('.', '')
    value = value.replace('March', 'Mar')
    value = value.replace('April', 'Apr')
    value = value.replace('June', 'Jun')
    value = value.replace('July', 'Jul')
    value = value.replace('Sept', 'Sep')
    value = value.replace('noon', '12:00 pm')

    if ':' not in value:
        if 'am' in value:
            value = value.replace(' am', ':00 am')
        elif 'pm' in value:
            value = value.replace(' pm', ':00 pm')

    return datetime.strptime(value, '%b %d, %Y, %H:%M %p')

def parse_version_url(url):
    crashReports = {}
    html_text = requests.get(url).text
    soup = BeautifulSoup(html_text, 'html.parser')

    table = soup.find("table", {"id": "data-table"}).tbody
    for tr in table.find_all("tr"):
        td_list = tr.find_all("td")
        crashName = td_list[0].a.text.strip()
        crashNumber = int(td_list[1].text.strip())
        firstCrashDate = convert_str_to_date(td_list[5].text.strip())
        lastCrashDate = convert_str_to_date(td_list[6].text.strip())
        crashReports[crashName] = [crashNumber, firstCrashDate, lastCrashDate]

    return crashReports

def parse_reports_and_get_most_recent_report_from_last_page(url):
    html_text = requests.get(url).text
    soup = BeautifulSoup(html_text, 'html.parser')

    count = 0
    os_tab = soup.find("table", {"id": "os_tab"}).tbody
    tr_list = os_tab.find_all("tr")
    for tr in tr_list:
        td_list = tr.find_all("td")
        count += int(td_list[1].text.strip())

    # There are 50 reports on each page.
    # Go to the last page based on the total count to get a recent report
    last_page = math.ceil( count / 50 )

    if last_page > 1:
        url = url + "?page=" + str(last_page)
        html_text = requests.get(url).text
        soup = BeautifulSoup(html_text, 'html.parser')

    reports = soup.find("div", {"id": "reports"}).tbody
    mostRecentVersion = ""
    report = ""

    for tr in reports.find_all("tr"):
        td_list = tr.find_all("td")

        # get most recent version
        currentVersion = td_list[2].text.strip().split(': ')[1]
        if currentVersion > mostRecentVersion:
            mostRecentVersion = currentVersion
            report = td_list[0].a.text.strip()

    return count, report, mostRecentVersion

def parse_details_and_get_reason(url):
    html_text = requests.get(url).text
    soup = BeautifulSoup(html_text, 'html.parser')

    reports = soup.find("div", {"id": "details"}).tbody
    tr_list = reports.find_all("tr")
    return tr_list[8].td.text.strip()

if __name__ == '__main__':

    version = sys.argv[1]

    crashes = parse_version_url(
            "https://crashreport.libreoffice.org/stats/version/" + version + "?limit=1000&days=30")

    print(str(len(crashes)) + " crash reports in version " + version)

    crashesInFile = []
    fileName = "crashes_" + version.replace(".", "_") + ".csv"
    print("Using " + fileName)

    if os.path.exists(fileName):
        with open(fileName, "r") as f:
            lines = f.readlines()
            for line in lines:
                crashesInFile.append(line.split(";")[0])

    with open(fileName, "a") as f:
        for k, v in crashes.items():
            # ignore unresolved crash signatures
            if len(k) < 254 and k not in crashesInFile and not k.lower().endswith('.dll') and \
                    not k.lower().endswith('.so') and ".so." not in k.lower():
                print("Parsing " + k)
                count, report, reportVersion = parse_reports_and_get_most_recent_report_from_last_page(
                        "https://crashreport.libreoffice.org/stats/signature/" + k)
                crashReason = parse_details_and_get_reason(
                        "https://crashreport.libreoffice.org/stats/crash_details/" + report)
                line = k + ";" + str(count) + ";" + v[1].strftime('%d/%m/%y') + ";" + v[2].strftime('%d/%m/%y') + \
                        ";" + report + ";" + reportVersion + ";" + crashReason + "\n"
                f.write(line)
                f.flush()

