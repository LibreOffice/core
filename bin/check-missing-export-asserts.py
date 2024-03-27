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
    os.environ["CPPUNITTRACE"] = "gdb -ex=run -ex=quit --args"
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
        if errout == '' or p.poll():
            break

    logger.info('Parsing output from ' + makeName)

    results = {}
    for line in err.strip().split('\n'):
        if not line:
            continue
        splitLine = line.split(' - ')
        typeCheck = splitLine[0]
        testName = splitLine[1]
        importNumber = splitLine[2]
        exportNumber = splitLine[3].strip()
        if importNumber != exportNumber:
            logger.info("WARNING: " + testName + " has different number of " + typeCheck + ". Import: " + \
                    importNumber + " Export: " + exportNumber)
        else:
            if testName.endswith('.odt') or testName.endswith('.ott'):
                if testName not in results:
                    results[testName] = {'pages': 0, 'checkPages': False, 'index': 0,
                        'shapes': 0, 'checkShapes': False}
                if typeCheck == 'PAGES':
                    results[testName]['pages'] = importNumber
                elif typeCheck == 'SHAPES':
                    results[testName]['shapes'] = importNumber

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
        elif 'getShapes' in line:
            if testName in results:
                results[testName]['checkShapes'] = True

    total = 0
    for i in results.values():
        if not i['checkPages'] and int(i['pages']) >= 1:
            total += 1
            lines.insert(i['index'] + 2, "    CPPUNIT_ASSERT_EQUAL(" + str(i['pages']) + ", getPages());\n")

        if not i['checkShapes'] and int(i['shapes']) >= 1:
            total += 1
            lines.insert(i['index'] + 2, "    CPPUNIT_ASSERT_EQUAL(" + str(i['shapes']) + ", getShapes());\n")

    logger.info(str(total) + ' missing asserts added in ' + fileName)

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
            modelTestLines.insert( i - 1, 'int nImportPages = getPages();int nImportShapes = getShapes();\n')
            modelTestLines.insert( i + 2, 'int nExportPages = getPages();int nExportShapes = getShapes();SAL_' + \
                'DEBUG("PAGES - " << filename << " - " << nImportPages << " - " << nExportPages);SAL_' + \
                'DEBUG("SHAPES - " << filename << " - " << nImportShapes << " - " << nExportShapes);\n')
            addText = False

    modelTestFile = open(modelTestPath, 'w')
    modelTestFile.write("".join(modelTestLines))
    modelTestFile.close()

def check_sw(repoPath):
    modelTestRelPath = '/sw/qa/unit/swmodeltestbase.cxx'

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
