#!/usr/bin/env python3

# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import re
import os
from subprocess import Popen, DEVNULL, PIPE
import logging
import sys

def start_logger():
    rootLogger = logging.getLogger()
    rootLogger.setLevel(os.environ.get("LOGLEVEL", "INFO"))

    logFormatter = logging.Formatter("%(asctime)s - %(message)s")
    fileHandler = logging.FileHandler("log.txt")
    fileHandler.setFormatter(logFormatter)
    rootLogger.addHandler(fileHandler)

    streamHandler = logging.StreamHandler(sys.stdout)
    rootLogger.addHandler(streamHandler)

    return rootLogger

def checkout_file(repoPath, filePath):
    p = Popen(['git', '-C', repoPath, 'checkout', repoPath + filePath],
            stdout=DEVNULL, stderr=DEVNULL)
    p.communicate()


def execute_make_and_parse_output(fileName, makeName):
    os.environ["CPPUNITTRACE"] = "gdb -ex run --args"
    p = Popen(['make', makeName],
            stdout=DEVNULL, stderr=PIPE)
    logger.info('Executing ' + makeName)

    err = ""
    while True:
        errout = p.stderr.readline().decode('utf-8').strip()
        if errout.startswith('debug:'):
            info = errout.split(': ')[1]
            err += info + '\n'
            logger.info(info)
        if 'OK' in errout or p.poll():
            break

    p.terminate()
    logger.info('Parsing output from ' + makeName)

    results = {}
    for line in err.strip().split('\n'):
        splitLine = line.split(' - ')
        if splitLine[0] == 'PAGES':
            testName = splitLine[1]
            importPages = splitLine[2]
            exporttPages = splitLine[3].strip()
            if importPages != exporttPages:
                logger.info("WARNING: " + testName + " has different number of pages. Import: " + \
                        importPages + " Export: " + exporttPages)
            else:
                if testName.endswith('.odt') or testName.endswith('.ott'):
                    results[testName] = {'pages': importPages, 'checkPages': False, 'index': 0}

    cxxFile = open(fileName, 'r')
    lines = cxxFile.readlines()
    cxxFile.close()

    for i in range(len(lines)):
        line = lines[i]

        if line.startswith('DECLARE'):
            try:
                testName = re.findall('"([^"]*)"', line)[0]
            except:
                #check in the next line when line is broken into 2 lines
                testName = re.findall('"([^"]*)"', lines[i + 1])[0]

            if testName in results:
                results[testName]['index'] = i

        elif 'getPages' in line:
            if testName in results:
                results[testName]['checkPages'] = True

    total = 0
    for i in results.values():
        if not i['checkPages']:
            total += 1
            lines.insert(i['index'] + 2, "    CPPUNIT_ASSERT_EQUAL(" + str(i['pages']) + ", getPages());\n")

    logger.info(str(total) + ' missing asserts found in ' + fileName)

    cxxFile = open(fileName, "w")
    cxxFile.write("".join(lines))
    cxxFile.close()

def insert_code_in_sw_model(repoPath, modelTestRelPath):
    modelTestPath = repoPath + modelTestRelPath
    modelTestFile = open(modelTestPath, 'r')
    modelTestLines = modelTestFile.readlines()
    modelTestFile.close()

    addText = False
    # Add code to check import and export pages
    for i in range(len(modelTestLines)):
        line = modelTestLines[i]
        if line.strip().startswith('void'):
            if 'executeLoadVerifyReloadVerify' in line or \
                    'executeLoadReloadVerify' in line:
                addText = True
            else:
                addText = False

        if addText and 'reload' in line:
            modelTestLines.insert( i - 1, 'int nImportPages = getPages();\n');
            modelTestLines.insert( i + 2, 'int nExportPages = getPages();SAL_' + \
                'DEBUG("PAGES - " << filename << " - " << nImportPages << " - " << nExportPages);\n');
            addText = False

    modelTestFile = open(modelTestPath, 'w')
    modelTestFile.write("".join(modelTestLines))
    modelTestFile.close()

def check_sw(repoPath):
    modelTestRelPath = '/sw/qa/inc/swmodeltestbase.hxx'

    checkout_file(repoPath, modelTestRelPath)
    insert_code_in_sw_model(repoPath, modelTestRelPath)

    os.chdir(repoPath)

    qaDir = os.path.join(repoPath, 'sw/qa/extras/')
    for dirName in os.listdir(qaDir):
        subdirName = os.path.join(qaDir, dirName)
        if not os.path.isdir(subdirName):
            continue

        for fileName in os.listdir(subdirName):
            if 'export' in fileName and fileName.endswith('.cxx'):
                fileNamePath = os.path.join(subdirName, fileName)

                p = Popen(['git', '-C', repoPath, 'diff', fileNamePath],
                        stdout=PIPE, stderr=DEVNULL)
                stdout = p.communicate()[0]
                if stdout != b'':
                    logger.info( fileNamePath + " has unstaged changes. Ignoring...")
                    continue
                makeName = 'CppunitTest_sw_' + os.path.basename(fileNamePath).split('.')[0]
                execute_make_and_parse_output(fileNamePath, makeName)

    checkout_file(repoPath, modelTestRelPath)

if __name__ == '__main__':
    logger = start_logger()

    repoPath = os.path.dirname(os.path.abspath(__file__)) + '/..'

    check_sw(repoPath)
