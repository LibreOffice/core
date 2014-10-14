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
needsCsvHeader = True

def processDirectory(rootDir):

  if needsCsvHeader:
    intermediateResult = "lastCommit\ttest name\tfiledatetime\tdump comment\tcount\n"
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

  fileDate = time.strftime('%m/%d/%Y %H:%M:%S', time.gmtime(os.path.getmtime(path)))

  result = lastCommitId + "\t" + testName + "\t" + fileDate + "\t" + message + "\t" + total

  return result

def getLastCommitId():

  stream = os.popen("git log")
  line = stream.readline()
  return line.replace("commit ","").replace("\n","")

def displayUsage():

  print
  print "Parses the callgrind results of make percheck"
  print
  print "Usage: bin/parse_perfcheck.py [targetFileName = perfcheckResult.csv] [sourceDirectory = ./workdir/CppunitTest]"
  print "default assumes running from core root directory"
  print

if __name__ == '__main__':

  #check args

  if len(sys.argv) < 3:
      if len(sys.argv) == 2:
          if sys.argv[1] == "--help":
            displayUsage()
            sys.exit(1)
          else:
            targetFileName = sys.argv[1]
            sourceDirectory = "./workdir/CppunitTest"
      elif len(sys.argv) == 1:
          targetFileName = "perfcheckResult.csv"
          sourceDirectory = "./workdir/CppunitTest"
      else:
          displayUsage()
          sys.exit(1)
  else:
      targetFileName = sys.argv[1]
      sourceDirectory = sys.argv[2]

  # check if sourceDirectorty exists
  if not os.path.isdir(sourceDirectory):
    print "sourceDirectorty %s not found - Aborting" % (sourceDirectory)
    sys.exit(1)

  # last commit Id
  lastCommitId = getLastCommitId()

  # needs header in csv file ?
  needsCsvHeader = not os.path.isfile(targetFileName)

  # call walker
  globalResult = processDirectory(sourceDirectory)

  print globalResult

  # write result
  fileResult = open(targetFileName,'a')
  fileResult.write(globalResult)
  fileResult.close()
