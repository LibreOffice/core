import traceback
from NoValidPathException import *
from com.sun.star.ucb import CommandAbortedException
from com.sun.star.uno import Exception as UnoException
from com.sun.star.awt.VclWindowPeerAttribute import OK, YES_NO
import types
from os import path as osPath

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
        except UnoException, exception:
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
        except UnoException, exception:
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
            while i < len(Template_internal):
                sPath = Template_internal[i]
                if sPath.startsWith("vnd."):
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

        except UnoException, exception:
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
        except CommandAbortedException, exception:
            sMsgNoDir = JavaTools.replaceSubString(sNoDirCreation, Path, "%1")
            SystemDialog.showMessageBox(xMSF, "ErrorBox", OK, sMsgNoDir)
            return False
        except com.sun.star.uno.Exception, unoexception:
            sMsgNoDir = JavaTools.replaceSubString(sNoDirCreation, Path, "%1")
            SystemDialog.showMessageBox(xMSF, "ErrorBox", OK, sMsgNoDir)
            return False

    '''
    checks if the root of a path exists. if the parameter
    xWindowPeer is not null then also the directory is
    created when it does not exists and the user
    '''

    @classmethod
    def PathisValid(self, xMSF, Path, sMsgFilePathInvalid,
        baskbeforeOverwrite):
        try:
            SubDirPath = ""
            bSubDirexists = True
            NewPath = Path
            xInterface = xMSF.createInstance(
                "com.sun.star.ucb.SimpleFileAccess")
            if baskbeforeOverwrite:
                if xInterface.exists(Path):
                    oResource = Resource.Resource_unknown(xMSF,
                        "ImportWizard", "imp")
                    sFileexists = oResource.getResText(1053)
                    NewString = JavaTools.convertfromURLNotation(Path)
                    sFileexists = JavaTools.replaceSubString(sFileexists,
                        NewString, "<1>")
                    sFileexists = JavaTools.replaceSubString(sFileexists,
                        str(13), "<CR>")
                    iLeave = SystemDialog.showMessageBox(xMSF, "QueryBox",
                        YES_NO, sFileexists)
                    if iLeave == 3:
                        return False

            DirArray = JavaTools.ArrayoutofString(Path, "/")
            MaxIndex = DirArray.length - 1
            if MaxIndex > 0:
                i = MaxIndex
                while i >= 0:
                    SubDir = DirArray[i]
                    SubLen = SubDir.length()
                    NewLen = NewPath.length()
                    RestLen = NewLen - SubLen
                    if RestLen > 0:
                        NewPath = NewPath.substring(0, NewLen - SubLen - 1)
                        if i == MaxIndex:
                            SubDirPath = NewPath

                        bexists = xSimpleFileAccess.exists(NewPath)
                        if bexists:
                            LowerCasePath = NewPath.toLowerCase()
                            bexists = (((LowerCasePath.equals("file:#/")) or
                                (LowerCasePath.equals("file:#")) or
                                (LowerCasePath.equals("file:/")) or
                                (LowerCasePath.equals("file:"))) == False)

                        if bexists:
                            if bSubDirexists == False:
                                bSubDiriscreated = createSubDirectory(xMSF,
                                    xSimpleFileAccess, SubDirPath)
                                return bSubDiriscreated

                            return True
                        else:
                            bSubDirexists = False

                    i -= 1

            SystemDialog.showMessageBox(xMSF, "ErrorBox", OK,
                sMsgFilePathInvalid)
            return False
        except com.sun.star.uno.Exception, exception:
            traceback.print_exc()
            SystemDialog.showMessageBox(xMSF, "ErrorBox", OK,
                sMsgFilePathInvalid)
            return False

    '''
    searches a directory for files which start with a certain
    prefix, and returns their URLs and document-titles.
    @param xMSF
    @param FilterName the prefix of the filename. a "-" is added to the prefix
    @param FolderName the folder (URL) to look for files...
    @return an array with two array members. The first one, with document
    titles, the second with the corresponding URLs.
    @deprecated please use the getFolderTitles() with ArrayList
    '''

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

            LocLayoutFiles[1] = sorted(NameVector)
            LocLayoutFiles[0] = sorted(TitleVector)

        except Exception, exception:
            traceback.print_exc()

        return LocLayoutFiles

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
        except com.sun.star.uno.Exception, e:
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

    @classmethod
    def getFolderTitles2(self, xMSF, _sStartFilterName, FolderName,
        _sEndFilterName=""):

        LocLayoutFiles = [[2],[]]
        if FolderName.size() == 0:
            raise NoValidPathException (None, "Path not given.");

        TitleVector = []
        URLVector = []
        xInterface = None
        try:
            xInterface = xMSF.createInstance(
                "com.sun.star.ucb.SimpleFileAccess")
        except com.sun.star.uno.Exception, e:
            traceback.print_exc()
            raise NoValidPathException (None, "Internal error.");

        j = 0
        while j < FolderName.size():
            sFolderName = FolderName.get(j)
            try:
                nameList = xInterface.getFolderContents(sFolderName, False)
                if _sStartFilterName == None or _sStartFilterName.equals(""):
                    _sStartFilterName = None
                else:
                    _sStartFilterName = _sStartFilterName + "-"

                fileName = ""
                i = 0
                while i < nameList.length:
                    fileName = self.getFilename(i)
                    if _sStartFilterName == None or \
                        fileName.startsWith(_sStartFilterName):
                        if _sEndFilterName.equals(""):
                            sTitle = getTitle(xMSF, nameList[i])
                        elif fileName.endsWith(_sEndFilterName):
                            fileName = fileName.replaceAll(
                                _sEndFilterName + "$", "")
                            sTitle = fileName
                        else:
                            # no or wrong (start|end) filter
                            continue

                        URLVector.add(nameList[i])
                        TitleVector.add(sTitle)

                    i += 1
            except CommandAbortedException, exception:
                traceback.print_exc()
            except com.sun.star.uno.Exception, e:
                pass

            j += 1
        LocNameList = [URLVector.size()]
        LocTitleList = [TitleVector.size()]
        # LLA: we have to check if this works
        URLVector.toArray(LocNameList)

        TitleVector.toArray(LocTitleList)

        LocLayoutFiles[1] = LocNameList
        LocLayoutFiles[0] = LocTitleList

        #COMMENTED
        #JavaTools.bubblesortList(LocLayoutFiles);
        return LocLayoutFiles

    def __init__(self, xmsf):
        #get a simple file access...
        self.fileAccess = xmsf.createInstance(
            "com.sun.star.ucb.SimpleFileAccess")
        #get the file identifier converter
        self.filenameConverter = xmsf.createInstance(
            "com.sun.star.ucb.FileContentProvider")

    def getURL(self, path, childPath=None):
        if childPath is not None:
            path = self.filenameConverter.getSystemPathFromFileURL(path)
            f = open(path,childPath)
        else:
            f = open(path)

        r = self.filenameConverter.getFileURLFromSystemPath(path,
             osPath.abspath(path))
        return r

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
        except CommandAbortedException, cax:
            traceback.print_exc()
        except com.sun.star.uno.Exception, ex:
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
        except CommandAbortedException, cax:
            pass
        except com.sun.star.uno.Exception, ex:
            pass

        return defe

    '''
    @author rpiterman
    @param filename
    @return
    '''

    def isDirectory(self, filename):
        try:
            return self.fileAccess.isFolder(filename)
        except CommandAbortedException, cax:
            pass
        except com.sun.star.uno.Exception, ex:
            pass

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
        except CommandAbortedException, cax:
            pass
        except com.sun.star.uno.Exception, ex:
            pass

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
        except CommandAbortedException, cax:
            traceback.print_exc()
        except com.sun.star.uno.Exception, ex:
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
        except CommandAbortedException, cax:
            pass
        except com.sun.star.uno.Exception, ex:
            pass

        return False

    def getLastModified(self, url):
        try:
            return self.fileAccess.getDateTimeModified(url)
        except CommandAbortedException, cax:
            pass
        except com.sun.star.uno.Exception, ex:
            pass

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
        s = getNewFile(parentDir, name, "")
        if mkdir(s):
            return s
        else:
            return None

    def getNewFile(self, parentDir, name, extension):
        i = 0
        tmp_do_var2 = True
        while tmp_do_var2:
            filename = filename(name, extension, (i + 1))
            u = getURL(parentDir, filename)
            url = u
            tmp_do_var2 = exists(url, True)
        return url

    @classmethod
    def filename(self, name, ext, i):
        stringI = ""
        stringExt = ""
        if i is not 0:
            stringI = str(i)
        if ext is not "":
            stringExt = "." + ext

        return name + stringI + StringExt

    def getSize(self, url):
        try:
            return self.fileAccess.getSize(url)
        except Exception, ex:
            return -1

    @classmethod
    def connectURLs(self, urlFolder, urlFilename):
        stringFolder = ""
        stringFileName = urlFilename
        if not urlFolder.endsWith("/"):
            stringFolder = "/"
        if urlFilename.startsWith("/"):
            stringFileName = urlFilename.substring(1)
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

