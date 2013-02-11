#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#
import traceback

'''
This class delivers static convenience methods
to use with ucb SimpleFileAccess service.
You can also instanciate the class, to encapsulate
some functionality of SimpleFileAccess. The instance
keeps a reference to an XSimpleFileAccess and an
XFileIdentifierConverter, saves the permanent
overhead of quering for those interfaces, and delivers
conveneince methods for using them.
These Convenince methods include mainly Exception-handling.
'''

class FileAccess(object):

    def __init__(self, xmsf):
        #get the file identifier converter
        self.filenameConverter = xmsf.createInstance(
            "com.sun.star.ucb.FileContentProvider")
            
    @classmethod
    def getFolderTitles(self, xMSF, FilterName, FolderName, resDict=None):
        #Returns and ordered dict containing the template's name and path
        
        locLayoutFiles = []
        try:
            xDocInterface = xMSF.createInstance(
                "com.sun.star.document.DocumentProperties")
            xInterface = xMSF.createInstance(
                "com.sun.star.ucb.SimpleFileAccess")
            nameList = xInterface.getFolderContents(FolderName, False)
            if FilterName is None or FilterName == "":
                FilterName = None
            else:
                FilterName += "-"
            
            locLayoutDict = {}
            for i in nameList:
                fileName = self.getFilename(i)
                if FilterName is None or fileName.startswith(FilterName):
                    xDocInterface.loadFromMedium(i, tuple())
                    if resDict is None:
                        title = xDocInterface.Title
                    else:
                        if xDocInterface.Title in resDict:
                            # localise string at runtime
                            title = resDict[xDocInterface.Title]
                        else:
                            title = xDocInterface.Title
                    locLayoutDict[title] = i
            
            #sort the dictionary and create a list containing the
            #keys list and the values list
            keysList = sorted(locLayoutDict.keys())
            valuesList= []
            for i in keysList:
                valuesList.append(locLayoutDict[i])
            locLayoutFiles.append(keysList)
            locLayoutFiles.append(valuesList)          
        except Exception:
            traceback.print_exc()

        return locLayoutFiles

    @classmethod
    def addPath(self, _sPath, _sPath2):
        if not _sPath.endsWith("/"):
            _sPath += "/"

        if _sPath2.startsWith("/"):
            _sPath2 = _sPath2.substring(1)

        sNewPath = _sPath + _sPath2
        return sNewPath

    @classmethod
    def getTitle(self, xMSF, _sFile):
        sTitle = ""
        try:
            xDocInterface = xMSF.createInstance(
                "com.sun.star.document.DocumentProperties")
            noArgs = []
            xDocInterface.loadFromMedium(_sFile, noArgs)
            sTitle = xDocInterface.getTitle()
        except Exception:
            traceback.print_exc()

        return sTitle

    def getPath(self, parentURL, childURL):
        string = ""
        if childURL is not None and childURL is not "":
            string = "/" + childURL
        return self.filenameConverter.getSystemPathFromFileURL(
            parentURL + string)

    '''
    return the filename out of a system-dependent path
    '''

    @classmethod
    def getPathFilename(self, path):
        return self.getFilename(path, File.separator)

    @classmethod
    def getFilename(self, path, pathSeparator = "/"):
        return path.split(pathSeparator)[-1]

    '''
    if the path points to file, gives the directory in which the file is.
    '''

    @classmethod
    def getParentDir(self, url):
        return url[:url.rfind("/")]

    @classmethod
    def connectURLs(self, urlFolder, urlFilename):
        stringFolder = ""
        stringFileName = urlFilename
        if not urlFolder.endswith("/"):
            stringFolder = "/"
        if urlFilename.startswith("/"):
            stringFileName = urlFilename[1:]
        return urlFolder + stringFolder + stringFileName
