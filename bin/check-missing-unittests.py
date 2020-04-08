#!/usr/bin/env python3

# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import os
import datetime
import subprocess

ignoredBugs = ['42949', '120703', '107197', '130977']

def main():
    results = {
        'hasTest': set(),
        'docx-export': {},
        'pptx-export': {},
        'xlsx-export': {}
    }

    repoPath = os.path.dirname(os.path.abspath(__file__)) + '/..'
    branch = subprocess.check_output(
            ['git', '-C', repoPath, 'rev-parse', '--abbrev-ref', 'HEAD'],
            stderr=subprocess.DEVNULL)
    output = subprocess.check_output(
            ['git', '-C', repoPath, 'log', '--since="2012-01-01', '--name-only' ,'--pretty=format:"%s"'],
            stderr=subprocess.DEVNULL)
    commits = output.decode('utf-8', 'ignore').split('\n\n')
    print("Analyzing " + str(len(commits)) + " commits in " + branch.decode().strip() + " branch...")

    for commit in reversed(commits):

        summary = commit.split('\n', 1)[0].lower()

        #Check summary has a bug id
        if 'tdf#' in summary:

            isIgnored = False
            for i in ignoredBugs:
                if i in summary:
                    isIgnored = True
            if isIgnored:
                continue

            if not summary.split('tdf#')[1][0].isdigit():
                continue

            bugId = ''.join(filter(str.isdigit, summary.split('tdf#')[1].split(' ')[0]))

            if bugId in results['hasTest']:
                continue

            changedFiles = commit.split('\n', 1)[1]
            if 'qa' in changedFiles:
                results['hasTest'].add(bugId)
            elif 'sw/source/filter/ww8/docx' in changedFiles:
                results['docx-export'][bugId] = summary
            elif 'sc/source/filter/excel/xe' in changedFiles:
                results['xlsx-export'][bugId] = summary
            elif 'oox/source/export/' in changedFiles:
                results['pptx-export'][bugId] = summary
            # Add others here

    print()
    print('{{TopMenu}}')
    print('{{Menu}}')
    print('{{Menu.Development}}')
    print()
    print('Generated on ' + str(datetime.datetime.now()))
    for k,v in results.items():
        if k == 'hasTest':
            continue

        print('\n=== ' + k + ' ===')
        for bugId, summary in v.items():
            if bugId not in results['hasTest']:
                print("* {} - [https://bugs.documentfoundation.org/show_bug.cgi?id={} {}]".format(
                    summary, bugId, bugId))
    print()
    print('[[Category:QA]][[Category:Development]]')

if __name__ == '__main__':
    main()
