#!/usr/bin/env python3

# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# Use this script to retrieve information from https://crashreport.libreoffice.org
# about a specific version of LibreOffice
# Usage sample: ./crashreportScraper.py 7.2.0.4

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
    # reset the time leaving the date
    value = ", ".join(value.split(", ")[:-1])
    dtDate = datetime.strptime(value, '%b %d, %Y')

    return dtDate.strftime('%y/%m/%d')

def parse_version_url(url):
    crashReports = {}

    try:
        html_text = requests.get(url, timeout=200).text
        soup = BeautifulSoup(html_text, 'html.parser')
    except requests.exceptions.Timeout:
        print("Timeout requesting " + url)
        sys.exit(1)

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
    try:
        html_text = requests.get(url, timeout=200).text
        soup = BeautifulSoup(html_text, 'html.parser')
    except requests.exceptions.Timeout:
        print("Timeout")
        raise

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
        try:
            html_text = requests.get(url, timeout=200).text
            soup = BeautifulSoup(html_text, 'html.parser')
        except requests.exceptions.Timeout:
            print("Timeout")
            raise

    reports = soup.find("div", {"id": "reports"}).tbody
    ID, currentID = "", ""
    version, currentVersion = "", ""
    OS, currentOS = "", ""

    tr_list = reports.find_all("tr")
    for tr in tr_list:
        td_list = tr.find_all("td")

        currentID = td_list[0].a.text.strip()
        currentVersion = td_list[2].text.strip().split(': ')[1]
        currentOS = td_list[3].text.strip()

        # get most recent version
        # symbols on linux are not very informative generally
        if currentOS == "windows" and currentVersion > version:
            version = currentVersion
            ID = currentID
            OS = currentOS

    if not version:
        version = currentVersion

    if not ID:
        ID = currentID

    if not OS:
        OS = currentOS

    return count, ID, version, OS

def parse_details_and_get_info(url, gitRepo):
    try:
        html_text = requests.get(url, timeout=200).text
        soup = BeautifulSoup(html_text, 'html.parser')
    except requests.exceptions.Timeout:
        print("Timeout")
        raise

    details = soup.find("div", {"id": "details"}).tbody
    tr_list = details.find_all("tr")
    reason = tr_list[8].td.text.strip()

    stack = ""
    codeLine = ""

    count = 0
    frames = soup.find("div", {"id": "frames"}).tbody
    for tr in frames.find_all("tr"):
        td_list = tr.find_all("td")
        source = td_list[3].text.strip()
        if source and count <= 10:
            source = source.replace("\\", "/").replace("C:/cygwin64/home/buildslave/source/libo-core/", "")
            stack += source + "\n"
            count += 1

            codeFile = source.split(":")[0]
            codeNumber = source.split(":")[1]
            try:
                with open(os.path.join(gitRepo, codeFile)) as f:
                    lines = f.readlines()
                    for index, line in enumerate(lines):
                        if index + 1 == int(codeNumber):
                            codeLine += line.strip() + "\n"
            except FileNotFoundError:
                codeLine += "\n"
                continue

    if stack:
        #multiline
        stack = "\"" + stack + "\""

    if codeLine:
        #multiline
        codeLine = "\"" + codeLine + "\""

    return reason, stack, codeLine


if __name__ == '__main__':

    version = sys.argv[1]

    crashes = parse_version_url(
            "https://crashreport.libreoffice.org/stats/version/" + version + "?limit=1000&days=30")

    gitRepo = os.path.dirname(os.path.realpath(__file__)) + "/../"

    print(str(len(crashes)) + " crash reports in version " + version)

    crashesInFile = []
    fileName = "crashes_" + version.replace(".", "_") + ".csv"
    print("Using " + fileName)

    bInsertHeader = False
    if os.path.exists(fileName):
        with open(fileName, "r") as f:
            lines = f.readlines()
            for line in lines:
                crashesInFile.append(line.split("\t")[0])
    else:
        bInsertHeader = True

    with open(fileName, "a") as f:
        if bInsertHeader:
            line = '\t'.join(["Name", "Count", "First report", "Last Report",
                "ID", "Version", "Reason", "OS", "Stack", "Code Lines" '\n'])
            f.write(line)
            f.flush()

        for k, lDate in crashes.items():
            if len(k) < 254 and k not in crashesInFile and '`' not in k:
                print("Parsing " + k)
                try:
                    crashCount, crashID, crashVersion, crashOS = parse_reports_and_get_most_recent_report_from_last_page(
                            "https://crashreport.libreoffice.org/stats/signature/" + k)
                    crashReason, crashStack, codeLine = parse_details_and_get_info(
                            "https://crashreport.libreoffice.org/stats/crash_details/" + crashID, gitRepo)
                    line = '\t'.join([k, str(crashCount), lDate[1], lDate[2],
                            crashID, crashVersion, crashReason, crashOS, crashStack, codeLine, '\n'])
                    f.write(line)
                    f.flush()
                except requests.exceptions.Timeout:
                    continue
