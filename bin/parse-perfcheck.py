#!/usr/bin/python

# This file is part of the LibreOffice project.
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

import sys
import os
import getopt
import csv


colsResult = {}
allTests = []

def parseFile(dirname, filename, lastCommit):

  curTestComment, total = None, None

  path = os.path.join(dirname, filename)

  trigger = "desc: Trigger: Client Request: "
  trigger_len = len(trigger)
  totals = "totals: "
  totals_len = len(totals)

  with open(path,'r') as callgrindFile:
     lines = callgrindFile.readlines()

     for line in lines:
         if line.startswith(trigger):
             curTestComment = line[trigger_len:].replace("\n","")
         elif line.startswith(totals):
             total = line[totals_len:].replace("\n","")

  if curTestComment is None or total is None:
    return None

  testName = os.path.basename(dirname).replace(".test.core","")

  lastCommitId, lastCommitDate = lastCommit
  if lastCommitId not in colsResult:
    colsResult[lastCommitId] = {}
    colsResult[lastCommitId]['date'] = lastCommitDate
    colsResult[lastCommitId]['values'] = {}

  colsResult[lastCommitId]['values'][curTestComment] = total

  return [lastCommitId, lastCommitDate, testName, curTestComment, total, filename]

def processDirectory(rootDir, needsCsvHeader, lastCommit):

    results = []

    if needsCsvHeader:
        results.append(["lastCommit", "lastCommitDate", "test filename", "dump comment", "count", "filename"])

    for dirName, subdirList, fileList in os.walk(rootDir):
        files = [f for f in fileList if f.startswith("callgrind.out.")]
        for fname in files:
            found = parseFile(dirName, fname, lastCommit)
            if found is not None:
                results.append(found)
    return results

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

Parses the callgrind results of make perfcheck

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

class WrongArguments(Exception):
    pass

def analyzeArgs(args):

    try:
        opts, args = getopt.getopt(args, 'x', [
            'csv-file=', 'source-directory=', 'alert-type=', 'alert-value=', 'help'])
    except getopt.GetoptError:
        raise WrongArguments

    targetFileName = "perfcheckResult.csv"
    sourceDirectory = "./workdir/CppunitTest"
    alertType = ""
    alertValue = 10

    for o, a in opts:
        if o == '--help':
            displayUsage()
            sys.exit()
        elif o == "--csv-file":
            targetFileName = a
        elif o == "--source-directory":
            sourceDirectory = a
        elif o == "--alert-type":
            alertType = a
        elif o == "--alert-value":
            alertValue = float(a)
        else:
            raise WrongArguments

    return targetFileName, sourceDirectory, alertType, alertValue

def readCsvFile(targetFilename):

    with open(targetFilename, 'r') as csvfile:
        reader = csv.reader(csvfile, delimiter="\t")
        # skip header
        next(reader)
        for line in reader:

            # do not process empty lines
            if not line:
                continue

            curId, curDate, curTestName, curTestComment, curValue = line

            if curTestComment not in allTests:
                allTests.append(curTestComment)

            if curId not in colsResult:
                colsResult[curId] = {}
                colsResult[curId]['date'] = curDate
                colsResult[curId]['values'] = {}

            colsResult[curId]['values'][curTestComment] = curValue

if __name__ == '__main__':

  #check args
  try:
      targetFileName, sourceDirectory, alertType, alertValue = analyzeArgs(sys.argv[1:])
  except WrongArguments:
      displayUsage()
      sys.exit(1)

  # check if sourceDirectorty exists
  if not os.path.isdir(sourceDirectory):
    print("sourceDirectory %s not found - Aborting" % (sourceDirectory))
    sys.exit(1)

  # read the complete CSV file
  if os.path.isfile(targetFileName):
    readCsvFile(targetFileName)
    needsCsvHeader = False
  else:
    needsCsvHeader = True

  # last commit Id
  lastCommitId, lastCommitDate = getLastCommitInfo()

  # walker through directory
  if lastCommitId not in colsResult:

    lastCommit = (lastCommitId, lastCommitDate)
    results = processDirectory(sourceDirectory, needsCsvHeader, lastCommit)
    ppResults = "\n".join(["\t".join(row) for row in results])

    print('\nNew results\n' + ppResults)

    # append raw result
    with open(targetFileName,'a') as csvfile:
        writer = csv.writer(csvfile, delimiter='\t')
        writer.writerows(results)
        print("\nCSV file written at " + targetFileName + '\n')

  else:
      print("\nCSV file up to date " + targetFileName + '\n')


  # build columned output

  # header
  mLine = '\t'.join(["commit", "date"] + allTests) + '\n'

  alertTest = {}

  with open(targetFileName + '.col','w') as fileResult:
      for k in colsResult:
          mLine += k + "\t" + colsResult[k]['date'] + "\t"
          for t in allTests:
              if t in colsResult[k]['values']:
                  mValue= colsResult[k]['values'][t]
                  if t not in alertTest:
                      alertTest[t] = {}
                  alertTest[t][colsResult[k]['date']] = mValue
              else:
                  mValue = ""
              mLine += mValue + "\t"
          mLine += "\n"

      # write columned result
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
