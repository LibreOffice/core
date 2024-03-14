#!/usr/bin/env python3

# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# Use this script to retrieve information from https://crashreport.libreoffice.org
# about a specific version of LibreOffice
# Usage sample: ./crashreportScraper.py --version 7.2.0.4 --repository /path/to/libreoffice/repository/

import argparse
import requests
from bs4 import BeautifulSoup
import sys
import os
import math
from datetime import datetime
import urllib.parse
import re

def convert_str_to_date(value):
    value = value.replace('.', '')
    value = value.replace('March', 'Mar')
    value = value.replace('April', 'Apr')
    value = value.replace('June', 'Jun')
    value = value.replace('July', 'Jul')
    value = value.replace('Sept', 'Sep')
    # reset the time leaving the date
    value = ", ".join(value.split(", ")[:-1])
    return datetime.strptime(value, '%b %d, %Y')

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
    try:
        os_tab = soup.find("table", {"id": "os_tab"}).tbody
    except AttributeError:
        print("os_tab not found")
        raise

    tr_list = os_tab.find_all("tr")
    for tr in tr_list:
        td_list = tr.find_all("td")
        count += int(td_list[1].text.strip())

    reports = soup.find("div", {"id": "reports"}).tbody
    ID, currentID = "", ""
    version, currentVersion = 0, 0
    OS, currentOS = "", ""

    tr_list = reports.find_all("tr")
    for tr in tr_list:
        td_list = tr.find_all("td")

        currentID = td_list[0].a.text.strip()
        currentVersion = int(''.join(re.findall("\d+", td_list[2].text)))
        currentOS = td_list[3].text.strip()

        # get most recent version
        # symbols on linux are not very informative generally
        if currentOS == "windows" and currentVersion > version:
            version = currentVersion
            ID = currentID
            OS = currentOS

    if not ID:
        ID = currentID

    if not OS:
        OS = currentOS

    return count, ID, OS

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
                            codeLine += line.strip().replace("\"", "'") + "\n"
            except FileNotFoundError:
                codeLine += "\n"
                continue

    if stack:
        #multiline
        stack = "\"" + stack + "\""

    if codeLine:
        #multiline
        codeLine = "\"" + codeLine + "\""

    metadata = soup.find("div", {"id": "metadata"}).tbody
    tr_list = metadata.find_all("tr")
    unoCommands = ""
    for tr in tr_list:
        if tr.th.text.strip() == "Last-4-Uno-Commands":
            unoCommands = tr.td.text.strip()

    return reason, stack, codeLine, unoCommands


if __name__ == '__main__':

    parser = argparse.ArgumentParser()

    parser.add_argument('--version', action='store', dest="version", required=True)
    parser.add_argument('--repository', action="store", dest="repository", required=True)

    args = parser.parse_args()

    crashes = parse_version_url(
            "https://crashreport.libreoffice.org/stats/version/" + args.version + "?limit=1000&days=30")

    print(str(len(crashes)) + " crash reports in version " + args.version)

    crashesInFile = []
    fileName = "crashes_" + args.version.replace(".", "_") + ".csv"
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
            line = '\t'.join(["Name", "Ratio", "Count", "First report", "Last Report",
                "ID", "Reason", "OS", "Stack", "Code Lines", "Last 4 UNO Commands", '\n'])
            f.write(line)
            f.flush()

        for k, lDate in crashes.items():
            if k not in crashesInFile:
                print("Parsing " + k)
                try:
                    crashCount, crashID, crashOS = parse_reports_and_get_most_recent_report_from_last_page(
                            "https://crashreport.libreoffice.org/stats/signature/" + urllib.parse.quote(k))
                    crashReason, crashStack, codeLine, unoCommands = parse_details_and_get_info(
                            "https://crashreport.libreoffice.org/stats/crash_details/" + crashID, args.repository)
                    ratio = round(crashCount / ((lDate[2] - lDate[1]).days + 1), 2)
                    line = '\t'.join([k, str(ratio), str(crashCount) , lDate[1].strftime('%y/%m/%d'), lDate[2].strftime('%y/%m/%d'),
                            crashID, crashReason, crashOS, crashStack, codeLine, unoCommands, '\n'])
                    f.write(line)
                    f.flush()
                except (requests.exceptions.Timeout, AttributeError):
                    continue
