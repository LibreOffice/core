#!/usr/bin/env python3

# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# Use this script to update
# https://wiki.documentfoundation.org/MissingUnitTests

import os
import datetime
import subprocess
import sys
import re
import json
import requests

def splitList(lst, n):
    for i in range(0, len(lst), n):
        yield lst[i:i + n]

def whiteboardNotes(whiteboard):
    if not whiteboard:
        return ''
    if ' ' in whiteboard:
        whiteboardList = reversed(whiteboard.split(' '))
        for w in whiteboardList:
            if w.startswith("unitTestNotes"):
                return w.split(':')[1]
    elif whiteboard.startswith("unitTestNotes"):
        return whiteboard.split(':')[1]

    return ''

def linesModified(commit_hash):
    repoPath = os.path.dirname(os.path.abspath(__file__)) + '/..'
    commits = subprocess.check_output(
            ['git', '-C', repoPath, 'show', commit_hash, '--shortstat'],
            stderr=subprocess.DEVNULL)
    linesmodified = commits.decode('utf-8', 'ignore').split('\n\n')
    stats = linesmodified[-1].strip("\n").split(",")
    insertions = 0
    deletions = 0
    # Make sure we have the file stats, a file stat must have
    # file(s) changed in it
    if "files changed" in linesmodified[-1] or "file changed" in linesmodified[-1]:
        fileschanged = re.findall(r'\b\d+\b', stats[0])[0]
        for element in stats:
           if "+" in element:
               insertions = re.findall(r'\b\d+\b', element)[0]
           elif "-" in element:
               deletions = re.findall(r'\b\d+\b', element)[0]
        return [fileschanged, insertions, deletions]
    else:
        return ["not found", "not found", "not found"]

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
            'format': {},
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
            ['git', '-C', repoPath, 'log', '--since="2012-01-01', '--name-only' ,'--pretty=format:"%s%n%ad%n%H"', '--date=format:"%Y/%m/%d"'],
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

            commitHash = commitInfo[2].strip('"')
            date = commitInfo[1].strip('"')
            infoList = [date, summary, commitHash]

            changedFiles = "".join(commitInfo[3:])
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

            elif 'svl/source/numbers/' in changedFiles:
                results['calc']['format'][bugId] = infoList

            # Keep the following if statements at the end

            elif 'sc/source/core/' in changedFiles:
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
    resultList = []
    fixList = []
    #Split the list into different chunks for the requests, otherwise it fails
    for chunk in splitList(listOfBugIdsWithoutTest, 50):
        urlGet = 'https://bugs.documentfoundation.org/rest/bug?id=' + ','.join(chunk)
        rGet = requests.get(urlGet)
        rawData = json.loads(rGet.text)
        rGet.close()
        bugzillaJson.extend(rawData['bugs'])

    for k,v in results.items():
        for k1, v1 in v.items():
            for bugId, info in v1.items():

                resolution = ''
                keywords = []
                priority = ''
                notes = ''
                for bug in bugzillaJson:
                    if str(bug['id']) == str(bugId):
                        resolution = bug['resolution']
                        keywords = bug['keywords']
                        priority = bug['priority']
                        notes = whiteboardNotes(bug['whiteboard'])
                        break

                # Only care about FIXED bugs
                # Ignore performance bugs and accessibility bugs
                if resolution and resolution == 'FIXED' and 'perf' not in keywords \
                        and 'accessibility' not in keywords:
                    stats = linesModified(info[2])
                    fixList.append({
                        "id": bugId,
                        "date": info[0],
                        "priority": priority.upper(),
                        "summary": info[1],
                        "maintopic":k,
                        "subtopic":k1,
                        "fileschanged": stats[0],
                        "insertions": stats[1],
                        "deletions": stats[2],
                        "notes": notes
                    })

    resultList.append([{
        "Generator": os.path.basename(sys.argv[0]),
        "Date": str(datetime.datetime.now()),
        "Commits": str(len(commits)),
        "Branch": branch.decode().strip(),
        "Hash": str(last_hash.decode().strip()),
    }])
    resultList.append(fixList)
    print(json.dumps(resultList))

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
