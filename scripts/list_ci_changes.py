#!/usr/bin/env python3
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at https://mozilla.org/MPL/2.0/.
#
# Lists your outgoing gerrit changes, showing if the change is waiting in the
# CI queue. Useful as otherwise the web browser would show this info in a huge
# tooltip that usually blinks + frequently reloads.
#
# The only optional parameter is a gerrit change integer, in that case it
# checks for that change, instead of all your own changes.
#

from html.parser import HTMLParser
import json
import re
import subprocess
import urllib.request
import sys


def getGerritChanges():
    ret = []

    p = subprocess.Popen(["ssh", "-p", "29418", "gerrit.collaboraoffice.com", "gerrit", "query", "--format=json", "status:open project:online owner:self"], stdout=subprocess.PIPE)
    lines = p.communicate()[0].decode('utf-8').splitlines()
    for line in lines:
        j = json.loads(line)
        if "url" in j.keys():
            ret.append(j)

    return ret


def downloadString(url):
    response = urllib.request.urlopen(url)
    data = response.read()
    return data.decode('utf-8')


def getCIChanges(changesHTML):
    """Returns a dict mapping queued Gerrit change URLs to their subjects
    (the GERRIT_CHANGE_SUBJECT value from the tooltip)."""
    class MyHTMLParser(HTMLParser):
        def __init__(self):
            HTMLParser.__init__(self)
            self.changes = {}

        def handle_starttag(self, tag, attrs):
            if tag == "a":
                for attrKey, attrValue in attrs:
                    if attrKey == "tooltip":
                        urlMatch = re.search(r"https://gerrit.collaboraoffice.com/c/online/\+/[0-9]+", attrValue)
                        if not urlMatch:
                            continue
                        subjectMatch = re.search(r"GERRIT_CHANGE_SUBJECT=([^\n]*)", attrValue)
                        subject = subjectMatch.group(1) if subjectMatch else ""
                        self.changes[urlMatch.group(0)] = subject

    parser = MyHTMLParser()
    parser.feed(changesHTML)
    return parser.changes

def main():
    if len(sys.argv) > 1:
        gerrit = [{"url": "https://gerrit.collaboraoffice.com/c/online/+/" + sys.argv[1], "branch": "unknown"}]
    else:
        gerrit = getGerritChanges()

    changesHTML = downloadString("https://cpci.cbg.collabora.co.uk:8080/")
    ci = getCIChanges(changesHTML)

    for gerritChange in gerrit:
        url = gerritChange["url"]
        # Prefer the subject from the gerrit query; fall back to the one in
        # the CI tooltip (useful when the script is invoked with an explicit
        # change number, which bypasses the gerrit query).
        subject = gerritChange.get("subject") or ci.get(url, "")
        line = url
        if subject:
            line += f' "{subject}"'
        line += " for " + gerritChange["branch"] + ": "
        if url in ci:
            line += "change is in the CI queue"
        else:
            line += "change is not in the CI queue"
        print(line)

if __name__ == '__main__':
    main()

# vim:set shiftwidth=4 softtabstop=4 expandtab:
