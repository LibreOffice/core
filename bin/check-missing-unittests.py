#!/usr/bin/env python3

# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import os
import datetime
import subprocess
import sys
import re
import json
import requests

def isOpen(status):
    return status == 'NEW' or status == 'ASSIGNED' or status == 'REOPENED'

def splitList(lst, n):
    for i in range(0, len(lst), n):
        yield lst[i:i + n]

def main(ignoredBugs):
    results = {
        'export': {
            'docx': {},
            'doc': {},
            'pptx': {},
            'xlsx': {},
            'xhtml': {},
            'html': {},
        },
        'writer': {
            'undo': {},
            'autoformat': {},
            'autocorrect': {},
            'others': {},
        },
        'calc': {
            'import': {},
            'others': {},
        },
        'impress': {
            'drawingml': {},
            'slidesorter': {},
            'others': {},
        },

    }
    hasTestSet = set()

    repoPath = os.path.dirname(os.path.abspath(__file__)) + '/..'
    branch = subprocess.check_output(
            ['git', '-C', repoPath, 'rev-parse', '--abbrev-ref', 'HEAD'],
            stderr=subprocess.DEVNULL)
    last_hash = subprocess.check_output(
            ['git', '-C', repoPath, 'rev-parse', 'HEAD'],
            stderr=subprocess.DEVNULL)
    output = subprocess.check_output(
            ['git', '-C', repoPath, 'log', '--since="2012-01-01', '--name-only' ,'--pretty=format:"%s%n%ad"', '--date=format:"%Y/%m/%d"'],
            stderr=subprocess.DEVNULL)
    commits = output.decode('utf-8', 'ignore').split('\n\n')

    for commit in reversed(commits):

        commitInfo = commit.split('\n')

        summary = commitInfo[0].strip('"').lower()

        # Check for bugIds in the summary. Ignore those with a '-' after the digits.
        # Those are used for file names ( e.g. tdf129410-1.ods )
        bugIds = re.findall("\\b(?:bug|fdo|tdf|lo)[#:]?(\\d+)(?!-)\\b", summary)
        if bugIds is None or len(bugIds) == 0:
            continue

        for bugId in bugIds:

            isIgnored = False
            for i in ignoredBugs:
                if i in summary:
                    isIgnored = True
            if isIgnored:
                continue

            if bugId in hasTestSet:
                continue

            date = commitInfo[1].strip('"')
            infoList = [date, summary]

            changedFiles = "".join(commitInfo[2:])
            if 'qa' in changedFiles:
                hasTestSet.add(bugId)
                continue

            elif 'sw/source/filter/ww8/docx' in changedFiles or \
                    'writerfilter/source/dmapper' in changedFiles or \
                    'starmath/source/ooxmlimport' in changedFiles:
                results['export']['docx'][bugId] = infoList

            elif 'sw/source/filter/ww8/ww8' in changedFiles:
                results['export']['doc'][bugId] = infoList

            elif 'sc/source/filter/excel/xe' in changedFiles:
                results['export']['xlsx'][bugId] = infoList

            elif 'oox/source/export/' in changedFiles:
                results['export']['pptx'][bugId] = infoList

            elif 'filter/source/xslt/odf2xhtml/export' in changedFiles:
                results['export']['xhtml'][bugId] = infoList

            elif 'sw/source/filter/html/' in changedFiles:
                results['export']['html'][bugId] = infoList

            elif 'sw/source/core/undo/' in changedFiles:
                results['writer']['undo'][bugId] = infoList

            elif 'sw/source/core/edit/autofmt' in changedFiles:
                results['writer']['autoformat'][bugId] = infoList

            elif 'sw/source/core/edit/acorrect' in changedFiles:
                results['writer']['autocorrect'][bugId] = infoList

            elif 'drawingml' in changedFiles:
                results['impress']['drawingml'][bugId] = infoList

            elif 'sd/source/ui/slidesorter/' in changedFiles:
                results['impress']['slidesorter'][bugId] = infoList

            elif 'sc/source/core/tool/interpr' in changedFiles:
                results['calc']['import'][bugId] = infoList

            # Keep the following if statements at the end

            elif 'sc/source/core/data/' in changedFiles:
                results['calc']['others'][bugId] = infoList

            elif 'sw/source/core/' in changedFiles:
                results['writer']['others'][bugId] = infoList

            elif 'sd/source/core/' in changedFiles:
                results['impress']['others'][bugId] = infoList

    listOfBugIdsWithoutTest = []
    for k,v in results.items():
        for k1, v1 in v.items():
            for bugId, info in v1.items():
                if bugId not in hasTestSet:
                    listOfBugIdsWithoutTest.append(bugId)

    bugzillaJson = []
    #Split the list into different chunks for the requests, otherwise it fails
    for chunk in splitList(listOfBugIdsWithoutTest, 50):
        urlGet = 'https://bugs.documentfoundation.org/rest/bug?id=' + ','.join(chunk)
        rGet = requests.get(urlGet)
        rawData = json.loads(rGet.text)
        rGet.close()
        bugzillaJson.extend(rawData['bugs'])

    print()
    print('{{TopMenu}}')
    print('{{Menu}}')
    print('{{Menu.Development}}')
    print()
    print('Date: ' + str(datetime.datetime.now()))
    print()
    print('Commits: ' + str(len(commits)))
    print()
    print('Branch: ' + branch.decode().strip())
    print()
    print('Hash: ' + str(last_hash.decode().strip()))

    for k,v in results.items():
        print('\n== ' + k + ' ==')
        for k1, v1 in v.items():
            print('\n=== ' + k1 + ' ===')
            for bugId, info in v1.items():

                status = ''
                keywords = []
                priority = ''
                for bug in bugzillaJson:
                    if str(bug['id']) == str(bugId):
                        status = bug['status']
                        keywords = bug['keywords']
                        priority = bug['priority']
                        break

                #Ignore open bugs and performance bugs
                if status and not isOpen(status) and 'perf' not in keywords:
                    print(
                        "# {} - [{}] {} - [https://bugs.documentfoundation.org/show_bug.cgi?id={} tdf#{}]".format(
                        info[0], priority.upper(), info[1], bugId, bugId))

    print('\n== ignored bugs ==')
    print(' '.join(ignoredBugs))
    print()
    print('[[Category:QA]][[Category:Development]]')

def usage():
    message = """usage: {program} [bugs to ignore (each one is one argument)]

Sample: {program} 10000 10001 10002"""
    print(message.format(program = os.path.basename(sys.argv[0])))

if __name__ == '__main__':

    args = set()
    if len(sys.argv) > 1:
        arg1 = sys.argv[1]
        if arg1 == '-h' or arg1 == "--help":
            usage()
            sys.exit(1)
        for i in sys.argv:
            if i.isdigit():
                args.add(i)

    main(sorted(args))
