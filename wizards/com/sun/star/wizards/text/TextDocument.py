import uno
from common.Desktop import Desktop
from com.sun.star.view.DocumentZoomType import ENTIRE_PAGE
from com.sun.star.beans.PropertyState import DIRECT_VALUE
from common.Helper import Helper
from document.OfficeDocument import OfficeDocument
import traceback
from text.ViewHandler import ViewHandler
from text.TextFieldHandler import TextFieldHandler
from com.sun.star.container import NoSuchElementException
from com.sun.star.lang import WrappedTargetException
from common.Configuration import Configuration
import time
from com.sun.star.util import DateTime

class TextDocument(object):

    def __init__(self, xMSF,listener=None,bShowStatusIndicator=None,
        FrameName=None,_sPreviewURL=None,_moduleIdentifier=None,
        _textDocument=None, xArgs=None):

        self.xMSF = xMSF
        self.xTextDocument = None

        if listener is not None:
            if FrameName is not None:
                '''creates an instance of TextDocument
                and creates a named frame.
                No document is actually loaded into this frame.'''
                self.xFrame = OfficeDocument.createNewFrame(
                    xMSF, listener, FrameName)
                return

            elif _sPreviewURL is not None:
                '''creates an instance of TextDocument by
                loading a given URL as preview'''
                self.xFrame = OfficeDocument.createNewFrame(xMSF, listener)
                self.xTextDocument = self.loadAsPreview(_sPreviewURL, True)

            elif xArgs is not None:
                '''creates an instance of TextDocument
                and creates a frame and loads a document'''
                self.xDesktop = Desktop.getDesktop(xMSF);
                self.xFrame = OfficeDocument.createNewFrame(xMSF, listener)
                self.xTextDocument = OfficeDocument.load(
                    xFrame, URL, "_self", xArgs);
                self.xWindowPeer = xFrame.getComponentWindow()
                self.m_xDocProps = self.xTextDocument.DocumentProperties
                CharLocale = Helper.getUnoStructValue(
                    self.xTextDocument, "CharLocale");
                return

            else:
                '''creates an instance of TextDocument from
                the desktop's current frame'''
                self.xDesktop = Desktop.getDesktop(xMSF);
                self.xFrame = self.xDesktop.getActiveFrame()
                self.xTextDocument = self.xFrame.getController().Model

        elif _moduleIdentifier is not None:
            try:
                '''create the empty document, and set its module identifier'''
                self.xTextDocument = xMSF.createInstance(
                    "com.sun.star.text.TextDocument")
                self.xTextDocument.initNew()
                self.xTextDocument.setIdentifier(_moduleIdentifier.Identifier)
                # load the document into a blank frame
                xDesktop = Desktop.getDesktop(xMSF)
                loadArgs = range(1)
                loadArgs[0] = "Model"
                loadArgs[0] = -1
                loadArgs[0] = self.xTextDocument
                loadArgs[0] = DIRECT_VALUE
                xDesktop.loadComponentFromURL(
                    "private:object", "_blank", 0, loadArgs)
                # remember some things for later usage
                self.xFrame = self.xTextDocument.CurrentController.Frame
            except Exception, e:
                traceback.print_exc()

        elif _textDocument is not None:
            '''creates an instance of TextDocument
            from a given XTextDocument'''
            self.xFrame = _textDocument.CurrentController.Frame
            self.xTextDocument = _textDocument
        if bShowStatusIndicator:
            self.showStatusIndicator()
        self.init()

    def init(self):
        self.xWindowPeer = self.xFrame.getComponentWindow()
        self.m_xDocProps = self.xTextDocument.getDocumentProperties()
        self.CharLocale = Helper.getUnoStructValue(
            self.xTextDocument, "CharLocale")
        self.xText = self.xTextDocument.Text

    def showStatusIndicator(self):
        self.xProgressBar = self.xFrame.createStatusIndicator()
        self.xProgressBar.start("", 100)
        self.xProgressBar.setValue(5)

    def loadAsPreview(self, sDefaultTemplate, asTemplate):
        loadValues = range(3)
        #      open document in the Preview mode
        loadValues[0] = uno.createUnoStruct(
            'com.sun.star.beans.PropertyValue')
        loadValues[0].Name = "ReadOnly"
        loadValues[0].Value = True
        loadValues[1] = uno.createUnoStruct(
            'com.sun.star.beans.PropertyValue')
        loadValues[1].Name = "AsTemplate"
        if asTemplate:
            loadValues[1].Value = True
        else:
            loadValues[1].Value = False

        loadValues[2] = uno.createUnoStruct(
            'com.sun.star.beans.PropertyValue')
        loadValues[2].Name = "Preview"
        loadValues[2].Value = True
        '''set the preview document to non-modified
        mode in order to avoid the 'do u want to save' box'''
        if self.xTextDocument is not None:
            try:
                self.xTextDocument.setModified(False)
            except PropertyVetoException, e1:
                traceback.print_exc()
        self.xTextDocument = OfficeDocument.load(
            self.xFrame, sDefaultTemplate, "_self", loadValues)
        self.DocSize = self.getPageSize()
        myViewHandler = ViewHandler(self.xTextDocument, self.xTextDocument)
        try:
            myViewHandler.setViewSetting(
                "ZoomType", uno.Any("short",ENTIRE_PAGE))
        except Exception, e:
            traceback.print_exc()
        myFieldHandler = TextFieldHandler(self.xMSF, self.xTextDocument)
        myFieldHandler.updateDocInfoFields()
        return self.xTextDocument

    def getPageSize(self):
        try:
            xNameAccess = self.xTextDocument.getStyleFamilies()
            xPageStyleCollection = xNameAccess.getByName("PageStyles")
            xPageStyle = xPageStyleCollection.getByName("First Page")
            return Helper.getUnoPropertyValue(xPageStyle, "Size")
        except Exception, exception:
            traceback.print_exc()
            return None

    '''creates an instance of TextDocument and creates a
    frame and loads a document'''

    def createTextCursor(self, oCursorContainer):
        xTextCursor = oCursorContainer.createTextCursor()
        return xTextCursor

    # Todo: This method is  unsecure because the last index is not necessarily the last section
    # Todo: This Routine should be  modified, because I cannot rely on the last Table in the document to be the last in the TextTables sequence
    # to make it really safe you must acquire the Tablenames before the insertion and after the insertion of the new Table. By comparing the
    # two sequences of tablenames you can find out the tablename of the last inserted Table
    # Todo: This method is  unsecure because the last index is not necessarily the last section

    def getCharWidth(self, ScaleString):
        iScale = 200
        self.xTextDocument.lockControllers()
        iScaleLen = ScaleString.length()
        xTextCursor = createTextCursor(self.xTextDocument.Text)
        xTextCursor.gotoStart(False)
        com.sun.star.wizards.common.Helper.setUnoPropertyValue(
            xTextCursor, "PageDescName", "First Page")
        xTextCursor.String = ScaleString
        xViewCursor = self.xTextDocument.CurrentController
        xTextViewCursor = xViewCursor.ViewCursor
        xTextViewCursor.gotoStart(False)
        iFirstPos = xTextViewCursor.Position.X
        xTextViewCursor.gotoEnd(False)
        iLastPos = xTextViewCursor.Position.X
        iScale = (iLastPos - iFirstPos) / iScaleLen
        xTextCursor.gotoStart(False)
        xTextCursor.gotoEnd(True)
        xTextCursor.String = ""
        unlockallControllers()
        return iScale

    def unlockallControllers(self):
        while self.xTextDocument.hasControllersLocked() == True:
            self.xTextDocument.unlockControllers()

    def refresh(self):
        self.xTextDocument.refresh()

    '''
    This method sets the Author of a Wizard-generated template correctly
    and adds a explanatory sentence to the template description.
    @param WizardName The name of the Wizard.
    @param TemplateDescription The old Description which is being
    appended with another sentence.
    @return void.
    '''

    def setWizardTemplateDocInfo(self, WizardName, TemplateDescription):
        try:
            xNA = Configuration.getConfigurationRoot(
                self.xMSF, "/org.openoffice.UserProfile/Data", False)
            gn = xNA.getByName("givenname")
            sn = xNA.getByName("sn")
            fullname = str(gn) + " " + str(sn)
            currentDate = DateTime()
            now = time.localtime(time.time())
            currentDate.Day = time.strftime("%d", now)
            currentDate.Year = time.strftime("%Y", now)
            currentDate.Month = time.strftime("%m", now)
            du = Helper.DateUtils(self.xMSF, self.xTextDocument)
            ff = du.getFormat(NumberFormatIndex.DATE_SYS_DDMMYY)
            myDate = du.format(ff, currentDate)
            xDocProps2 = self.xTextDocument.getDocumentProperties()
            xDocProps2.setAuthor(fullname)
            xDocProps2.setModifiedBy(fullname)
            description = xDocProps2.getDescription()
            description = description + " " + TemplateDescription
            description = JavaTools.replaceSubString(
                description, WizardName, "<wizard_name>")
            description = JavaTools.replaceSubString(
                description, myDate, "<current_date>")
            xDocProps2.setDescription(description)
        except NoSuchElementException, e:
            # TODO Auto-generated catch block
            traceback.print_exc()
        except WrappedTargetException, e:
            # TODO Auto-generated catch block
            traceback.print_exc()
        except Exception, e:
            # TODO Auto-generated catch block
            traceback.print_exc()

    '''
    removes an arbitrary Object which supports the  'XTextContent' interface
    @param oTextContent
    @return
    '''

    def removeTextContent(self, oTextContent):
        try:
            self.xText.removeTextContent(oxTextContent)
            return True
        except NoSuchElementException, e:
            traceback.print_exc()
            return False

    '''
    Apparently there is no other way to get the
    page count of a text document other than using a cursor and
    making it jump to the last page...
    @param model the document model.
    @return the page count of the document.
    '''

    def getPageCount(self, model):
        xController = model.getCurrentController()
        xPC = xController.getViewCursor()
        xPC.jumpToLastPage()
        return xPC.getPage()

    def getFrameByName(self, sFrameName, xTD):
        if xTD.TextFrames.hasByName(sFrameName):
            return xTD.TextFrames.getByName(sFrameName)

        return None

    '''
    Possible Values for "OptionString" are: "LoadCellStyles",
    "LoadTextStyles", "LoadFrameStyles",
    "LoadPageStyles", "LoadNumberingStyles", "OverwriteStyles"
    '''
