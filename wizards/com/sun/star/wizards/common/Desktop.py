import uno
import traceback
from com.sun.star.frame.FrameSearchFlag import ALL, PARENT
from com.sun.star.util import URL
from com.sun.star.i18n.KParseTokens import ANY_LETTER_OR_NUMBER, ASC_UNDERSCORE
from NoValidPathException import *
from com.sun.star.uno import Exception as UnoException


class Desktop(object):

    @classmethod
    def getDesktop(self, xMSF):
        xDesktop = None
        if xMSF is not None:
            try:
                xDesktop = xMSF.createInstance( "com.sun.star.frame.Desktop")
            except UnoException, exception:
                traceback.print_exc()
        else:
            print "Can't create a desktop. null pointer !"

        return xDesktop

    @classmethod
    def getActiveFrame(self, xMSF):
        xDesktop = self.getDesktop(xMSF)
        return xDesktop.getActiveFrame()

    @classmethod
    def getActiveComponent(self, _xMSF):
        xFrame = self.getActiveFrame(_xMSF)
        return xFrame.getController().getModel()

    @classmethod
    def getActiveTextDocument(self, _xMSF):
        xComponent = getActiveComponent(_xMSF)
        return xComponent #Text

    @classmethod
    def getActiveSpreadsheetDocument(self, _xMSF):
        xComponent = getActiveComponent(_xMSF)
        return xComponent

    @classmethod
    def getDispatcher(self, xMSF, xFrame, _stargetframe, oURL):
        try:
            oURLArray = range(1)
            oURLArray[0] = oURL
            xDispatch = xFrame.queryDispatch(oURLArray[0], _stargetframe, ALL)
            return xDispatch
        except UnoException, e:
            e.printStackTrace(System.out)

        return None

    @classmethod
    def getDispatchURL(self, xMSF, _sURL):
        try:
            oTransformer = xMSF.createInstance("com.sun.star.util.URLTransformer")
            oURL = range(1)
            oURL[0] = com.sun.star.util.URL.URL()
            oURL[0].Complete = _sURL
            xTransformer.parseStrict(oURL)
            return oURL[0]
        except UnoException, e:
            e.printStackTrace(System.out)

        return None

    @classmethod
    def dispatchURL(self, xMSF, sURL, xFrame, _stargetframe):
        oURL = getDispatchURL(xMSF, sURL)
        xDispatch = getDispatcher(xMSF, xFrame, _stargetframe, oURL)
        dispatchURL(xDispatch, oURL)

    @classmethod
    def dispatchURL(self, xMSF, sURL, xFrame):
        dispatchURL(xMSF, sURL, xFrame, "")

    @classmethod
    def dispatchURL(self, _xDispatch, oURL):
        oArg = range(0)
        _xDispatch.dispatch(oURL, oArg)

    @classmethod
    def connect(self, connectStr):
        localContext = uno.getComponentContext()
        resolver = localContext.ServiceManager.createInstanceWithContext(
				        "com.sun.star.bridge.UnoUrlResolver", localContext )
        ctx = resolver.resolve( connectStr )
        orb = ctx.ServiceManager
        return orb

    @classmethod
    def checkforfirstSpecialCharacter(self, _xMSF, _sString, _aLocale):
        try:
            nStartFlags = ANY_LETTER_OR_NUMBER + ASC_UNDERSCORE
            ocharservice = _xMSF.createInstance("com.sun.star.i18n.CharacterClassification")
            aResult = ocharservice.parsePredefinedToken(KParseType.IDENTNAME, _sString, 0, _aLocale, nStartFlags, "", nStartFlags, " ")
            return aResult.EndPos
        except UnoException, e:
            e.printStackTrace(System.out)
            return -1

    @classmethod
    def removeSpecialCharacters(self, _xMSF, _aLocale, _sname):
        snewname = _sname
        i = 0
        while i < snewname.length():
            i = Desktop.checkforfirstSpecialCharacter(_xMSF, snewname, _aLocale)
            if i < snewname.length():
                sspecialchar = snewname.substring(i, i + 1)
                snewname = JavaTools.replaceSubString(snewname, "", sspecialchar)

        return snewname

    '''
    Checks if the passed Element Name already exists in the  ElementContainer. If yes it appends a
    suffix to make it unique
    @param xElementContainer
    @param sElementName
    @return a unique Name ready to be added to the container.
    '''

    @classmethod
    def getUniqueName(self, xElementContainer, sElementName):
        bElementexists = True
        i = 1
        sIncSuffix = ""
        BaseName = sElementName
        while bElementexists == True:
            bElementexists = xElementContainer.hasByName(sElementName)
            if bElementexists == True:
                i += 1
                sElementName = BaseName + str(i)

        if i > 1:
            sIncSuffix = str(i)

        return sElementName + sIncSuffix

    '''
    Checks if the passed Element Name already exists in the list If yes it appends a
    suffix to make it unique
    @param _slist
    @param _sElementName
    @param _sSuffixSeparator
    @return a unique Name not being in the passed list.
    '''

    @classmethod
    def getUniqueNameList(self, _slist, _sElementName, _sSuffixSeparator):
        a = 2
        scompname = _sElementName
        bElementexists = True
        if _slist == None:
            return _sElementName

        if _slist.length == 0:
            return _sElementName

        while bElementexists == True:
            i = 0
            while i < _slist.length:
                if JavaTools.FieldInList(_slist, scompname) == -1:
                    return scompname

                i += 1
            scompname = _sElementName + _sSuffixSeparator + (a + 1)
        return ""

    '''
    @deprecated  use Configuration.getConfigurationRoot() with the same parameters instead
    @param xMSF
    @param KeyName
    @param bForUpdate
    @return
    '''

    @classmethod
    def getRegistryKeyContent(self, xMSF, KeyName, bForUpdate):
        try:
            aNodePath = range(1)
            oConfigProvider = xMSF.createInstance("com.sun.star.configuration.ConfigurationProvider")
            aNodePath[0] = uno.createUnoStruct('com.sun.star.beans.PropertyValue')
            aNodePath[0].Name = "nodepath"
            aNodePath[0].Value = KeyName
            if bForUpdate:
                return oConfigProvider.createInstanceWithArguments("com.sun.star.configuration.ConfigurationUpdateAccess", aNodePath)
            else:
                return oConfigProvider.createInstanceWithArguments("com.sun.star.configuration.ConfigurationAccess", aNodePath)

        except UnoException, exception:
            exception.printStackTrace(System.out)
            return None

class OfficePathRetriever:

    def OfficePathRetriever(self, xMSF):
        try:
            TemplatePath = FileAccess.getOfficePath(xMSF, "Template", "share", "/wizard")
            UserTemplatePath = FileAccess.getOfficePath(xMSF, "Template", "user", "")
            BitmapPath = FileAccess.combinePaths(xMSF, TemplatePath, "/../wizard/bitmap")
            WorkPath = FileAccess.getOfficePath(xMSF, "Work", "", "")
        except NoValidPathException, nopathexception:
            pass

    @classmethod
    def getTemplatePath(self, _xMSF):
        sTemplatePath = ""
        try:
            sTemplatePath = FileAccess.getOfficePath(_xMSF, "Template", "share", "/wizard")
        except NoValidPathException, nopathexception:
            pass
        return sTemplatePath

    @classmethod
    def getUserTemplatePath(self, _xMSF):
        sUserTemplatePath = ""
        try:
            sUserTemplatePath = FileAccess.getOfficePath(_xMSF, "Template", "user", "")
        except NoValidPathException, nopathexception:
            pass
        return sUserTemplatePath

    @classmethod
    def getBitmapPath(self, _xMSF):
        sBitmapPath = ""
        try:
            sBitmapPath = FileAccess.combinePaths(_xMSF, getTemplatePath(_xMSF), "/../wizard/bitmap")
        except NoValidPathException, nopathexception:
            pass

        return sBitmapPath

    @classmethod
    def getWorkPath(self, _xMSF):
        sWorkPath = ""
        try:
            sWorkPath = FileAccess.getOfficePath(_xMSF, "Work", "", "")

        except NoValidPathException, nopathexception:
            pass

        return sWorkPath

    @classmethod
    def createStringSubstitution(self, xMSF):
        xPathSubst = None
        try:
            xPathSubst = xMSF.createInstance("com.sun.star.util.PathSubstitution")
        except com.sun.star.uno.Exception, e:
            e.printStackTrace()

        if xPathSubst != None:
            return xPathSubst
        else:
            return None

    '''This method searches (and hopefully finds...) a frame
    with a componentWindow.
    It does it in three phases:
    1. Check if the given desktop argument has a componentWindow.
    If it is null, the myFrame argument is taken.
    2. Go up the tree of frames and search a frame with a component window.
    3. Get from the desktop all the components, and give the first one
    which has a frame.
    @param xMSF
    @param myFrame
    @param desktop
    @return
    @throws NoSuchElementException
    @throws WrappedTargetException
    '''

    @classmethod
    def findAFrame(self, xMSF, myFrame, desktop):
        if desktop == None:
            desktop = myFrame
            #we go up in the tree...

        while desktop != None and desktop.getComponentWindow() == None:
            desktop = desktop.findFrame("_parent", FrameSearchFlag.PARENT)
        if desktop == None:
            e = Desktop.getDesktop(xMSF).getComponents().createEnumeration()
            while e.hasMoreElements():
                xModel = (e.nextElement()).getObject()
                xFrame = xModel.getCurrentController().getFrame()
                if xFrame != None and xFrame.getComponentWindow() != None:
                    return xFrame

        return desktop
