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

def main(ignoredBugs):
    results = {
        'export': {
            'docx': {},
            'pptx': {},
            'xlsx': {}
        }
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
            ['git', '-C', repoPath, 'log', '--since="2012-01-01', '--name-only' ,'--pretty=format:"%s"'],
            stderr=subprocess.DEVNULL)
    commits = output.decode('utf-8', 'ignore').split('\n\n')

    for commit in reversed(commits):

        summary = commit.split('\n', 1)[0].lower()

        #Check summary has a bug id
        if 'tdf#' in summary or 'fdo#' in summary:

            isIgnored = False
            for i in ignoredBugs:
                if i in summary:
                    isIgnored = True
            if isIgnored:
                continue

            if 'tdf#' in summary:
                if not summary.split('tdf#')[1][0].isdigit():
                    continue
                bugId = ''.join(filter(str.isdigit, summary.split('tdf#')[1].split(' ')[0]))
            elif 'fdo#' in summary:
                if not summary.split('fdo#')[1][0].isdigit():
                    continue
                bugId = ''.join(filter(str.isdigit, summary.split('fdo#')[1].split(' ')[0]))


            if bugId in hasTestSet:
                continue

            changedFiles = commit.split('\n', 1)[1]
            if 'qa' in changedFiles:
                hasTestSet.add(bugId)
                continue
            elif 'sw/source/filter/ww8/docx' in changedFiles:
                results['export']['docx'][bugId] = summary
            elif 'sc/source/filter/excel/xe' in changedFiles:
                results['export']['xlsx'][bugId] = summary
            elif 'oox/source/export/' in changedFiles:
                results['export']['pptx'][bugId] = summary
            # Add others here

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
            for bugId, summary in v1.items():
                if bugId not in hasTestSet:
                    print(
                        "* {} - [https://bugs.documentfoundation.org/show_bug.cgi?id={} tdf#{}]".format(
                        summary, bugId, bugId))
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
