#!/usr/bin/python

# This file is part of the LibreOffice project.
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import sys
import os
import time

parseTrigger = "desc: Trigger: Client Request: "
parseTotal = "totals: "

separator = os.path.sep

lastCommitId = ""
lastCommitDate = ""
needsCsvHeader = True

def processDirectory(rootDir):

  if needsCsvHeader:
    intermediateResult = "lastCommit\ttest name\tdatetime\tdump comment\tcount\n"
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

  message = ""
  total = ""

  for line in lines:
    if line.startswith(parseTrigger):
      message = line[len(parseTrigger):]
    elif line.startswith(parseTotal):
      total = line[len(parseTotal):]

  callgrindFile.close()

  if message == "" and total == "0\n":
    return ""

  dirs = dirname.split(separator)
  currentTest = dirs[-1:]
  testName = currentTest[0].replace(".test.core","")

  message = message.replace("\n","")

  result = lastCommitId + "\t" + testName + "\t" + lastCommitDate + "\t" + message + "\t" + total

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

  print
  print "Parses the callgrind results of make percheck"
  print
  print "Usage: bin/parse_perfcheck.py [targetFileName = perfcheckResult.csv] [sourceDirectory = ./workdir/CppunitTest] ['--alert-previous'||'--alert-global'] [alertValue = 10]"
  print
  print "default assumes running from core root directory"
  print "per test columned output written at targetFileName +'.col'"
  print
  print "--alert-previous : compare to the previous entry of the test"
  print "--alert-global : compare to the first entry entry of the test"
  print "alertValue : value in %"
  print

def analyzeArgs(args):

    isValid = True

    alertType = ""
    alertValue = 10

    if len(args) < 3:
      if len(args) == 2:
          if args[1] == "--help":
            isValid = False
          else:
            targetFileName = args[1]
            sourceDirectory = "./workdir/CppunitTest"
      elif len(args) == 1:
          targetFileName = "perfcheckResult.csv"
          sourceDirectory = "./workdir/CppunitTest"
      else:
          isValid = False
    else:

      targetFileName = args[1]
      sourceDirectory = args[2]

      if len(args) > 3:
        alertType = args[3]
        if alertType not in ['--alert-previous','--alert-global']:
          isValid = False

      if len(args) > 4:
          alertValue = args[4]

    return isValid, targetFileName, sourceDirectory, alertType, alertValue

if __name__ == '__main__':

  #check args
  isOk, targetFileName, sourceDirectory, alertType, alertValue = analyzeArgs(sys.argv)

  if not isOk:
    displayUsage()
    sys.exit(1)

  # check if sourceDirectorty exists
  if not os.path.isdir(sourceDirectory):
    print "sourceDirectory %s not found - Aborting" % (sourceDirectory)
    sys.exit(1)

  # last commit Id
  lastCommitId, lastCommitDate = getLastCommitInfo()

  # needs header in csv file ? yes if new
  needsCsvHeader = not os.path.isfile(targetFileName)

  # walker through directory
  globalResult = processDirectory(sourceDirectory)

  print '\n' + globalResult

  # append raw result
  fileResult = open(targetFileName,'a')
  fileResult.write(globalResult)
  fileResult.close()

  print "\nCSV file written at " + targetFileName + '\n'

# format result by column

  # read the complete CSV file
  fileResult = open(targetFileName,'r')
  lines = fileResult.readlines()
  fileResult.close

  lines = lines[1:]

  colsResult = {}
  allTests = []

  for line in lines:

    spliter = line.replace('\n','').split('\t')
    curId = spliter[0]
    curDate = spliter[2]
    curTestName = spliter[1]
    curTestComment = spliter[3]
    curValue = spliter[4]

    if not curTestComment in allTests:
      allTests.append(curTestComment)

    if not colsResult.has_key(curId):
      colsResult[curId] = {}
      colsResult[curId]['date'] = curDate
      colsResult[curId]['values'] = {}

    colsResult[curId]['values'][curTestComment] = curValue

# build columned output

  # header
  mLine = "commit\tdate\t"
  for t in allTests:
    mLine += t + "\t"
  mLine += '\n'

  # commits
  alertTest = {}

  for k in colsResult.keys():

    mLine += k + "\t" + colsResult[k]['date'] + "\t"

    for t in allTests:

      if colsResult[k]['values'].has_key(t):
        mValue= colsResult[k]['values'][t]

        if not t in alertTest:
          alertTest[t] = {}
        alertTest[t][colsResult[k]['date']] = mValue

      else:
        mValue = ""

      mLine += mValue + "\t"

    mLine += "\n"

  # write columned result
  fileResult = open(targetFileName + '.col','w')
  fileResult.write(mLine)
  fileResult.close()

  print "Columned file written at " + targetFileName + '.col\n'

# check for Alerts

  if alertType == "":
    sys.exit(1)

  alertResult = ""

  for t in alertTest:

    testDict = alertTest[t]

    # sort
    keylist = testDict.keys()
    keylist.sort()
    maxVal = float(testDict[keylist[-1]])
    minVal = 0

    if alertType == "--alert-previous":
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
    print "!!!!!!!! ALERT !!!!!!!\n\n" + alertResult
