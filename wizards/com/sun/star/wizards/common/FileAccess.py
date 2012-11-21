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
import types
from os import path as osPath
from .NoValidPathException import NoValidPathException

from com.sun.star.ucb import CommandAbortedException
from com.sun.star.awt.VclWindowPeerAttribute import OK, YES_NO

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

    @classmethod
    def addOfficePath(self, xMSF, sPath, sAddPath):
        xSimpleFileAccess = None
        ResultPath = getOfficePath(xMSF, sPath, xSimpleFileAccess)
        '''
        As there are several conventions about the look of Url
        (e.g. with " " or with "%20") you cannot make a
        simple String comparison to find out, if a path
        is already in "ResultPath
        '''
        PathList = JavaTools.ArrayoutofString(ResultPath, ";")
        MaxIndex = PathList.length - 1
        CompAddPath = JavaTools.replaceSubString(sAddPath, "", "/")
        i = 0
        while i <= MaxIndex:
            CurPath = JavaTools.convertfromURLNotation(PathList[i])
            CompCurPath = JavaTools.replaceSubString(CurPath, "", "/")
            if CompCurPath.equals(CompAddPath):
                return

            i += 1
        ResultPath += ";" + sAddPath
        return

    @classmethod
    def deleteLastSlashfromUrl(self, _sPath):
        if _sPath.endswith("/"):
            return _sPath[:-1]
        else:
            return _sPath

    '''
    Further information on arguments value see in OO Developer Guide,
    chapter 6.2.7
    @param xMSF
    @param sPath
    @param xSimpleFileAccess
    @return the respective path of the office application.
    A probable following "/" at the end is trimmed.
    '''

    @classmethod
    def getOfficePath(self, xMSF, sPath, xSimpleFileAccess):
        try:
            ResultPath = ""
            xInterface = xMSF.createInstance("com.sun.star.util.PathSettings")
            ResultPath = str(Helper.getUnoPropertyValue(xInterface, sPath))
            ResultPath = self.deleteLastSlashfromUrl(ResultPath)
            return ResultPath
        except Exception:
            traceback.print_exc()
            return ""

    '''
    Further information on arguments value see in OO Developer Guide,
    chapter 6.2.7
    @param xMSF
    @param sPath
    @param sType use "share" or "user". Set to ""
    f not needed eg for the WorkPath;
    In the return Officepath a possible slash at the end is cut off
    @param sSearchDir
    @return
    @throws NoValidPathException
    '''

    @classmethod
    def getOfficePath2(self, xMSF, sPath, sType, sSearchDir):
        #This method currently only works with sPath="Template"
        bexists = False
        try:
            xPathInterface = xMSF.createInstance(
                "com.sun.star.util.PathSettings")
            ResultPath = ""
            ReadPaths = ()
            xUcbInterface = xMSF.createInstance(
                "com.sun.star.ucb.SimpleFileAccess")
            Template_writable = xPathInterface.getPropertyValue(
                sPath + "_writable")
            Template_internal = xPathInterface.getPropertyValue(
                sPath + "_internal")
            Template_user = xPathInterface.getPropertyValue(
                sPath + "_user")
            if type(Template_internal) is not types.InstanceType:
                ReadPaths = ReadPaths + Template_internal
            if type(Template_user) is not types.InstanceType:
                ReadPaths = ReadPaths + Template_user
            ReadPaths = ReadPaths + (Template_writable,)
            if sType.lower() == "user":
                ResultPath = Template_writable
                bexists = True
            else:
                #find right path using the search sub path
                for i in ReadPaths:
                    tmpPath = i + sSearchDir
                    if xUcbInterface.exists(tmpPath):
                        ResultPath = i
                        bexists = True
                        break

            ResultPath = self.deleteLastSlashfromUrl(ResultPath)
        except Exception:
            traceback.print_exc()
            ResultPath = ""

        if not bexists:
            raise NoValidPathException (xMSF, "");

        return ResultPath

    @classmethod
    def isPathValid(self, xMSF, _sPath):
        bExists = False
        try:
            xUcbInterface = xMSF.createInstance(
                "com.sun.star.ucb.SimpleFileAccess")
            bExists = xUcbInterface.exists(_sPath)
        except Exception:
            traceback.print_exc()

        return bExists

    @classmethod
    def combinePaths(self, xMSF, _sFirstPath, _sSecondPath):
        bexists = False
        ReturnPath = ""
        try:
            xUcbInterface = xMSF.createInstance(
                "com.sun.star.ucb.SimpleFileAccess")
            ReturnPath = _sFirstPath + _sSecondPath
            bexists = xUcbInterface.exists(ReturnPath)
        except Exception:
            traceback.print_exc()
            return ""

        if not bexists:
            raise NoValidPathException (xMSF, "");

        return ReturnPath

    @classmethod
    def getFolderTitles(self, xMSF, FilterName, FolderName, resDict=None):
        #Returns and ordered dict containing the template's name and path
        
        LocLayoutFiles = {}
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
                    LocLayoutFiles[title] = i

        except Exception:
            traceback.print_exc()

        #TODO: return it sorted
        return LocLayoutFiles

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

    def __init__(self, xmsf):
        #get a simple file access...
        self.fileAccess = xmsf.createInstance(
            "com.sun.star.ucb.SimpleFileAccess")
        #get the file identifier converter
        self.filenameConverter = xmsf.createInstance(
            "com.sun.star.ucb.FileContentProvider")

    def getURL(self, path, childPath=None):
        try:
            f = open(path, 'w')

            r = self.filenameConverter.getFileURLFromSystemPath(path,
                 osPath.abspath(path))
            return r
        except Exception:
            traceback.print_exc()
            return None

    def getPath(self, parentURL, childURL):
        string = ""
        if childURL is not None and childURL is not "":
            string = "/" + childURL
        return self.filenameConverter.getSystemPathFromFileURL(
            parentURL + string)

    '''
    @author rpiterman
    @param filename
    @return the extension of the given filename.
    '''

    @classmethod
    def getExtension(self, filename):
        p = filename.indexOf(".")
        if p == -1:
            return ""
        else:
            while p > -1:
                filename = filename.substring(p + 1)
                p = filename.indexOf(".")

        return filename

    '''
    @author rpiterman
    @param s
    @return
    '''

    def mkdir(self, s):
        try:
            self.fileAccess.createFolder(s)
            return True
        except Exception:
            traceback.print_exc()

        return False

    '''
    @author rpiterman
    @param filename
    @param def what to return in case of an exception
    @return true if the given file exists or not.
    if an exception accures, returns the def value.
    '''

    def exists(self, filename, defe):
        try:
            return self.fileAccess.exists(filename)
        except Exception:
            traceback.print_exc()

        return defe

    '''
    @author rpiterman
    @param filename
    @return
    '''

    def isDirectory(self, filename):
        try:
            return self.fileAccess.isFolder(filename)
        except Exception:
            traceback.print_exc()

        return False

    '''
    lists the files in a given directory
    @author rpiterman
    @param dir
    @param includeFolders
    @return
    '''

    def listFiles(self, dir, includeFolders):
        try:
            return self.fileAccess.getFolderContents(dir, includeFolders)
        except Exception:
            traceback.print_exc()

        return range(0)

    '''
    @author rpiterman
    @param file
    @return
    '''

    def delete(self, file):
        try:
            self.fileAccess.kill(file)
            return True
        except Exception:
            traceback.print_exc()

        return False


    '''
    return the filename out of a system-dependent path
    @param path
    @return
    '''

    @classmethod
    def getPathFilename(self, path):
        return self.getFilename(path, File.separator)

    '''
    @author rpiterman
    @param path
    @param pathSeparator
    @return
    '''

    @classmethod
    def getFilename(self, path, pathSeparator = "/"):
        return path.split(pathSeparator)[-1]

    @classmethod
    def getBasename(self, path, pathSeparator):
        filename = self.getFilename(path, pathSeparator)
        sExtension = getExtension(filename)
        basename = filename.substring(0, filename.length() - \
            (sExtension.length() + 1))
        return basename

    '''
    @author rpiterman
    @param source
    @param target
    @return
    '''

    def copy(self, source, target):
        try:
            self.fileAccess.copy(source, target)
            return True
        except Exception:
            traceback.print_exc()

        return False

    def getLastModified(self, url):
        try:
            return self.fileAccess.getDateTimeModified(url)
        except Exception:
            traceback.print_exc()

        return None

    '''
    @param url
    @return the parent dir of the given url.
    if the path points to file, gives the directory in which the file is.
    '''

    @classmethod
    def getParentDir(self, url):
        while url[-1] == "/":
            url = hello[:-1]
        return url[:url.rfind("/")]

    def createNewDir(self, parentDir, name):
        s = self.getNewFile(parentDir, name, "")
        if self.mkdir(s):
            return s
        else:
            return None

    def getNewFile(self, parentDir, name, extension):
        i = 0
        temp = True
        while temp:
            filename = self.filename(name, extension, i)
            url = parentDir + "/" + filename
            temp = self.exists(url, True)
            i += 1
        return url

    @classmethod
    def filename(self, name, ext, i):
        stringI = ""
        stringExt = ""
        if i != 0:
            stringI = str(i)
        if ext !=  "":
            stringExt = "." + ext

        return name + stringI + stringExt

    @classmethod
    def connectURLs(self, urlFolder, urlFilename):
        stringFolder = ""
        stringFileName = urlFilename
        if not urlFolder.endswith("/"):
            stringFolder = "/"
        if urlFilename.startswith("/"):
            stringFileName = urlFilename[1:]
        return urlFolder + stringFolder + stringFileName
