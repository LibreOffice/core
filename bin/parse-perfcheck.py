#!/usr/bin/python

# This file is part of the LibreOffice project.
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import sys
import os

parseTrigger = "desc: Trigger: Client Request: "
parseTotal = "totals: "

separator = os.path.sep

lastCommitId = ""
lastCommitDate = ""
needsCsvHeader = True # needs header in csv file ? yes if new

colsResult = {}
allTests = []

def processDirectory(rootDir):

  if needsCsvHeader:
    intermediateResult = "lastCommit\tlastCommitDate\ttest filename\tdump comment\tcount\n"
  else:
   intermediateResult = ""

  for dirName, subdirList, fileList in os.walk(rootDir):

    files = [ fi for fi in fileList if fi.startswith("callgrind.out.") ]
    for fname in files:
        found = parseFile(dirName, fname)
        if found != "":
          intermediateResult += found

  return intermediateResult

def parseFile(dirname, filename):

  path = dirname + separator + filename
  callgrindFile = open(path,'r')
  lines = callgrindFile.readlines()

  curTestComment = ""
  total = "0"

  for line in lines:
    if line.startswith(parseTrigger):
      curTestComment = line[len(parseTrigger):].replace("\n","")
    elif line.startswith(parseTotal):
      total = line[len(parseTotal):].replace("\n","")

  callgrindFile.close()

  if curTestComment == "":
    return ""

  if total == "0": # should not occur, btw
    return ""

  dirs = dirname.split(separator)
  currentTest = dirs[-1:]
  testName = currentTest[0].replace(".test.core","")

  if lastCommitId not in colsResult:
    colsResult[lastCommitId] = {}
    colsResult[lastCommitId]['date'] = lastCommitDate
    colsResult[lastCommitId]['values'] = {}

  colsResult[lastCommitId]['values'][curTestComment] = total

  result = lastCommitId + "\t" + lastCommitDate + "\t" + testName + "\t" + curTestComment + "\t" + total + "\n"

  return result

def getLastCommitInfo():

  stream = os.popen("git log --date=iso")
  line = stream.readline()
  commitId = line.replace("commit ","").replace("\n","")
  line = stream.readline()
  line = stream.readline()
  commitDate = line.replace("Date: ","").replace("\n","").strip()

  return commitId, commitDate

def displayUsage():

  usage = """

Parses the callgrind results of make percheck

Arguments :

  --csv-file\t\t the target CSV file - new or containing previous tests - default : perfcheckResult.csv
  --source-directory\t directory containing make perfcheck output - default : ./workdir/CppunitTest
  --alert-type\t\t mode for calculating alerts - valid values : previous first
  --alert-value\t\t alert threshold in % - default = 10

  --help\t\t this message

Columned output is dumped into csv-file + ".col"

Alerts, if any, are displayed in standard output

"""
  print(usage)

def analyzeArgs(args):

    isValid = True

    targetFileName = "perfcheckResult.csv"
    sourceDirectory = "./workdir/CppunitTest"
    alertType = ""
    alertValue = 10

    if "--help" in args:
      isValid = False

    if isValid:

      for arg in args[1:]:

        found = False

        if arg.startswith("--csv-file"):
          spliter = arg.split("=")
          if spliter[1] != "":
            targetFileName = spliter[1]
            found = True

        elif arg.startswith("--source-directory"):
          spliter = arg.split("=")
          if spliter[1] != "":
            sourceDirectory = spliter[1]
            found = True

        elif arg.startswith("--alert-type"):
          spliter = arg.split("=")
          if spliter[1] in ['previous','first']:
            alertType = spliter[1]
            found = True
          else:
            isValid = False

        elif arg.startswith("--alert-value"):
          spliter = arg.split("=")
          if spliter[1] != "":
            alertValue = float(spliter[1])
            found = True

        isValid = isValid and found

    return isValid, targetFileName, sourceDirectory, alertType, alertValue

def readCsvFile():

    fileResult = open(targetFileName,'r')
    lines = fileResult.readlines()
    fileResult.close

    lines = lines[1:] #skip header

    for line in lines:

      if line.strip() != "": # do not process empty lines

        spliter = line.replace('\n','').split('\t')
        curId = spliter[0]
        curDate = spliter[1]
        curTestName = spliter[2]
        curTestComment = spliter[3]
        curValue = spliter[4]

        if curTestComment not in allTests:
          allTests.append(curTestComment)

        if curId not in colsResult:
          colsResult[curId] = {}
          colsResult[curId]['date'] = curDate
          colsResult[curId]['values'] = {}

        colsResult[curId]['values'][curTestComment] = curValue

if __name__ == '__main__':

  #check args
  isOk, targetFileName, sourceDirectory, alertType, alertValue = analyzeArgs(sys.argv)

  if not isOk:
    displayUsage()
    sys.exit(1)

  # check if sourceDirectorty exists
  if not os.path.isdir(sourceDirectory):
    print("sourceDirectory %s not found - Aborting" % (sourceDirectory))
    sys.exit(1)

  # read the complete CSV file
  if os.path.isfile(targetFileName):
    readCsvFile()
    needsCsvHeader = False

  # last commit Id
  lastCommitId, lastCommitDate = getLastCommitInfo()

  # walker through directory
  if not lastCommitId in colsResult:

    newResult = processDirectory(sourceDirectory)

    print('\nNew results\n' + newResult)

    # append raw result
    with open(targetFileName,'a') as fileResult:
      fileResult.write(newResult)

    print("\nCSV file written at " + targetFileName + '\n')

  else:
    print("\nCSV file up to date " + targetFileName + '\n')


  # build columned output

  # header
  mLine = '\t'.join(["commit", "date"] + allTests) + '\n'

  alertTest = {}

  for k in colsResult:

    mLine += k + "\t" + colsResult[k]['date'] + "\t"

    for t in allTests:

      if t in colsResult[k]['values']:
        mValue= colsResult[k]['values'][t]

        if not t in alertTest:
          alertTest[t] = {}
        alertTest[t][colsResult[k]['date']] = mValue

      else:
        mValue = ""

      mLine += mValue + "\t"

    mLine += "\n"

  # write columned result
    with open(targetFileName + '.col','w') as fileResult:
      fileResult.write(mLine)

  print("Columned file written at " + targetFileName + '.col\n')

  # check for Alerts

  if alertType == "":
    sys.exit(1)

  alertResult = ""

  for t in alertTest:

    testDict = alertTest[t]

    # sort
    keylist = sorted(testDict.keys())
    maxVal = float(testDict[keylist[-1]])
    minVal = 0

    if alertType == "previous":
      if len(keylist) > 1:
        minVal = float(testDict[keylist[-2]])

    else:
      minVal = float(testDict[keylist[0]])

    if minVal != 0:
      delta = 100 * ((maxVal-minVal)/minVal)
    else:
      delta = 0

    if delta > float(alertValue):
      alertResult +=  t + "\t" + "{:.2f}".format(delta) + " %\n"

  if alertResult != "":
    print("!!!!!!!! ALERT !!!!!!!\n")
    print(alertResult)
