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
from .NoValidPathException import *

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
    '''
    @param xMSF
    @param sPath
    @param sAddPath
    '''

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
        except Exception, exception:
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
        except Exception, exception:
            traceback.print_exc()
            ResultPath = ""

        if not bexists:
            raise NoValidPathException (xMSF, "");

        return ResultPath

    @classmethod
    def getOfficePaths(self, xMSF, _sPath, sType, sSearchDir):
        #This method currently only works with sPath="Template"
        aPathList = []
        Template_writable = ""
        try:
            xPathInterface = xMSF.createInstance(
                "com.sun.star.util.PathSettings")
            Template_writable = xPathInterface.getPropertyValue(
                _sPath + "_writable")
            Template_internal = xPathInterface.getPropertyValue(
                _sPath + "_internal")
            Template_user = xPathInterface.getPropertyValue(_sPath + "_user")
            i = 0
            for i in Template_internal:
                if i.startsWith("vnd."):
                    # if there exists a language in the directory,
                    # we try to add the right language
                    sPathToExpand = sPath.substring(len("vnd.sun.star.Expand:"))
                    xExpander = Helper.getMacroExpander(xMSF)
                    sPath = xExpander.expandMacros(sPathToExpand)

                sPath = checkIfLanguagePathExists(xMSF, sPath)
                aPathList.add(sPath)
                i += 1
            i = 0
            while i < Template_user.length:
                aPathList.add(Template_user[i])
                i += 1
            aPathList.add(Template_writable)

        except Exception, exception:
            traceback.print_exc()
        return aPathList

    @classmethod
    def checkIfLanguagePathExists(self, _xMSF, _sPath):
        try:
            defaults = _xMSF.createInstance("com.sun.star.text.Defaults")
            aLocale = Helper.getUnoStructValue(defaults, "CharLocale")
            if aLocale == None:
                java.util.Locale.getDefault()
                aLocale = com.sun.star.lang.Locale.Locale()
                aLocale.Country = java.util.Locale.getDefault().getCountry()
                aLocale.Language = java.util.Locale.getDefault().getLanguage()
                aLocale.Variant = java.util.Locale.getDefault().getVariant()

            sLanguage = aLocale.Language
            sCountry = aLocale.Country
            sVariant = aLocale.Variant
            # de-DE-Bayrisch
            aLocaleAll = StringBuffer.StringBuffer()
            aLocaleAll.append(sLanguage).append('-').append(sCountry).append('-').append(sVariant)
            sPath = _sPath + "/" + aLocaleAll.toString()
            xInterface = _xMSF.createInstance(
                "com.sun.star.ucb.SimpleFileAccess")
            if xInterface.exists(sPath):
                # de-DE
                return sPath

            aLocaleLang_Country = StringBuffer.StringBuffer()
            aLocaleLang_Country.append(sLanguage).append('-').append(sCountry)
            sPath = _sPath + "/" + aLocaleLang_Country.toString()
            if xInterface.exists(sPath):
                # de
                return sPath

            aLocaleLang = StringBuffer.StringBuffer()
            aLocaleLang.append(sLanguage)
            sPath = _sPath + "/" + aLocaleLang.toString()
            if xInterface.exists(sPath):
                # the absolute default is en-US or en
                return sPath

            sPath = _sPath + "/en-US"
            if xInterface.exists(sPath):
                return sPath

            sPath = _sPath + "/en"
            if xInterface.exists(sPath):
                return sPath

        except com.sun.star.uno.Exception, e:
            pass

        return _sPath

    @classmethod
    def combinePaths2(self, xMSF, _aFirstPath, _sSecondPath):
        i = 0
        while i < _aFirstPath.size():
            sOnePath = _aFirstPath.get(i)
            sOnePath = addPath(sOnePath, _sSecondPath)
            if isPathValid(xMSF, sOnePath):
                _aFirstPath.add(i, sOnePath)
                _aFirstPath.remove(i + 1)
            else:
                _aFirstPath.remove(i)
                i -= 1

            i += 1

    @classmethod
    def isPathValid(self, xMSF, _sPath):
        bExists = False
        try:
            xUcbInterface = xMSF.createInstance(
                "com.sun.star.ucb.SimpleFileAccess")
            bExists = xUcbInterface.exists(_sPath)
        except Exception, exception:
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
        except Exception, exception:
            traceback.print_exc()
            return ""

        if not bexists:
            raise NoValidPathException (xMSF, "");

        return ReturnPath

    @classmethod
    def createSubDirectory(self, xMSF, xSimpleFileAccess, Path):
        sNoDirCreation = ""
        try:
            oResource = Resource.Resource_unknown(xMSF, "ImportWizard", "imp")
            if oResource != None:
                sNoDirCreation = oResource.getResText(1050)
                sMsgDirNotThere = oResource.getResText(1051)
                sQueryForNewCreation = oResource.getResText(1052)
                OSPath = JavaTools.convertfromURLNotation(Path)
                sQueryMessage = JavaTools.replaceSubString(sMsgDirNotThere,
                    OSPath, "%1")
                sQueryMessage = sQueryMessage + (char)
                13 + sQueryForNewCreation
                icreate = SystemDialog.showMessageBox(xMSF, "QueryBox",
                    YES_NO, sQueryMessage)
                if icreate == 2:
                    xSimpleFileAccess.createFolder(Path)
                    return True

            return False
        except Exception:
            sMsgNoDir = JavaTools.replaceSubString(sNoDirCreation, Path, "%1")
            SystemDialog.showMessageBox(xMSF, "ErrorBox", OK, sMsgNoDir)
            return False

    @classmethod
    def getFolderTitles(self, xMSF, FilterName, FolderName):
        LocLayoutFiles = [[2],[]]
        try:
            xDocInterface = xMSF.createInstance(
                "com.sun.star.document.DocumentProperties")
            xInterface = xMSF.createInstance(
                "com.sun.star.ucb.SimpleFileAccess")
            nameList = xInterface.getFolderContents(FolderName, False)
            TitleVector = []
            NameVector = []
            if FilterName is None or FilterName == "":
                FilterName = None
            else:
                FilterName = FilterName + "-"
            fileName = ""
            NameVectorAppend = NameVector.append
            TitleVectorAppend = TitleVector.append
            for i in nameList:
                fileName = self.getFilename(i)
                if FilterName is None or fileName.startswith(FilterName):
                    xDocInterface.loadFromMedium(i, tuple())
                    NameVectorAppend(i)
                    TitleVectorAppend(xDocInterface.Title)

            LocLayoutFiles[1] = NameVector
            LocLayoutFiles[0] = TitleVector

        except Exception, exception:
            traceback.print_exc()

        return self.__bubblesortList(LocLayoutFiles)

    '''
    This function bubble sorts an array of with 2 dimensions.
    The default sorting order is the first dimension
    Only if sort2ndValue is True the second dimension is
    the relevant for the sorting order
    '''

    @classmethod
    def __bubblesortList(self, SortList):
        SortCount = len(SortList[0])
        DimCount = len(SortList)
        for i in xrange(SortCount):
            for t in xrange(SortCount - i - 1):
                if SortList[0][t] > SortList[0][t + 1]:
                    for k in xrange(DimCount):
                        DisplayDummy = SortList[k][t];
                        SortList[k][t] = SortList[k][t + 1];
                        SortList[k][t + 1] = DisplayDummy
        return SortList
    '''
    We search in all given path for a given file
    @param _sPath
    @param _sPath2
    @return
    '''

    @classmethod
    def addPath(self, _sPath, _sPath2):
        if not _sPath.endsWith("/"):
            _sPath += "/"

        if _sPath2.startsWith("/"):
            _sPath2 = _sPath2.substring(1)

        sNewPath = _sPath + _sPath2
        return sNewPath

    @classmethod
    def getPathFromList(self, xMSF, _aList, _sFile):
        sFoundFile = ""
        try:
            xInterface = xMSF.createInstance(
                "com.sun.star.ucb.SimpleFileAccess")
            i = 0
            while i < _aList.size():
                sPath = _aList.get(i)
                sPath = addPath(sPath, _sFile)
                if xInterface.exists(sPath):
                    sFoundFile = sPath

                i += 1
        except Exception, e:
            pass

        return sFoundFile

    @classmethod
    def getTitle(self, xMSF, _sFile):
        sTitle = ""
        try:
            xDocInterface = xMSF.createInstance(
                "com.sun.star.document.DocumentProperties")
            noArgs = []
            xDocInterface.loadFromMedium(_sFile, noArgs)
            sTitle = xDocInterface.getTitle()
        except Exception, e:
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

    def getSize(self, url):
        try:
            return self.fileAccess.getSize(url)
        except Exception, ex:
            return -1

    @classmethod
    def connectURLs(self, urlFolder, urlFilename):
        stringFolder = ""
        stringFileName = urlFilename
        if not urlFolder.endswith("/"):
            stringFolder = "/"
        if urlFilename.startswith("/"):
            stringFileName = urlFilename[1:]
        return urlFolder + stringFolder + stringFileName

    @classmethod
    def getDataFromTextFile(self, _xMSF, _filepath):
        sFileData = None
        try:
            oDataVector = []
            oSimpleFileAccess = _xMSF.createInstance(
                "com.sun.star.ucb.SimpleFileAccess")
            if oSimpleFileAccess.exists(_filepath):
                xInputStream = oSimpleFileAccess.openFileRead(_filepath)
                oTextInputStream = _xMSF.createInstance(
                    "com.sun.star.io.TextInputStream")
                oTextInputStream.setInputStream(xInputStream)
                while not oTextInputStream.isEOF():
                    oDataVector.addElement(oTextInputStream.readLine())
                oTextInputStream.closeInput()
                sFileData = [oDataVector.size()]
                oDataVector.toArray(sFileData)

        except Exception, e:
            traceback.print_exc()

        return sFileData

    '''
    shortens a filename to a user displayable representation.
    @param path
    @param maxLength
    @return
    '''

    @classmethod
    def getShortFilename(self, path, maxLength):
        firstPart = 0
        if path.length() > maxLength:
            if path.startsWith("/"):
                # unix
                nextSlash = path.indexOf("/", 1) + 1
                firstPart = Math.min(nextSlash, (maxLength - 3) / 2)
            else:
                #windows
                firstPart = Math.min(10, (maxLength - 3) / 2)

            s1 = path.substring(0, firstPart)
            s2 = path.substring(path.length() - (maxLength - (3 + firstPart)))
            return s1 + "..." + s2
        else:
            return path

