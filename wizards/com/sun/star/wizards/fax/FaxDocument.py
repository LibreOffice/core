import uno
from text.TextDocument import *
from text.TextSectionHandler import TextSectionHandler
from text.TextFieldHandler import TextFieldHandler
from common.Configuration import Configuration
from common.PropertyNames import PropertyNames

from com.sun.star.text.ControlCharacter import PARAGRAPH_BREAK
from com.sun.star.style.ParagraphAdjust import CENTER
from com.sun.star.text.PageNumberType import CURRENT
from com.sun.star.style.NumberingType import ARABIC

class FaxDocument(TextDocument):

    def __init__(self, xMSF, listener):
        super(FaxDocument,self).__init__(xMSF, listener, None,
            "WIZARD_LIVE_PREVIEW")
        self.keepLogoFrame = True
        self.keepTypeFrame = True

    def switchElement(self, sElement, bState):
        try:
            mySectionHandler = TextSectionHandler(self.xMSF,
                self.xTextDocument)
            oSection = \
                mySectionHandler.xTextDocument.TextSections.getByName(sElement)
            Helper.setUnoPropertyValue(oSection, "IsVisible", bState)
        except Exception, exception:
            traceback.print_exc()

    def updateDateFields(self):
        FH = TextFieldHandler(self.xTextDocument, self.xTextDocument)
        FH.updateDateFields()

    def switchFooter(self, sPageStyle, bState, bPageNumber, sText):
        if self.xTextDocument is not None:
            self.xTextDocument.lockControllers()
            try:

                xPageStyleCollection = \
                    self.xTextDocument.StyleFamilies.getByName("PageStyles")
                xPageStyle = xPageStyleCollection.getByName(sPageStyle)

                if bState:
                    xPageStyle.setPropertyValue("FooterIsOn", True)
                    xFooterText = Helper.getUnoPropertyValue(xPageStyle, "FooterText")
                    xFooterText.String = sText

                    if bPageNumber:
                        #Adding the Page Number
                        myCursor = xFooterText.Text.createTextCursor()
                        myCursor.gotoEnd(False)
                        xFooterText.insertControlCharacter(myCursor,
                            PARAGRAPH_BREAK, False)
                        myCursor.setPropertyValue("ParaAdjust", CENTER )

                        xPageNumberField = self.xTextDocument.createInstance(
                            "com.sun.star.text.TextField.PageNumber")
                        xPageNumberField.setPropertyValue("SubType", CURRENT)
                        uno.invoke(xPageNumberField, "setPropertyValue",
                            ("NumberingType", uno.Any("short",ARABIC)))
                        xFooterText.insertTextContent(xFooterText.End,
                            xPageNumberField, False)
                else:
                    Helper.setUnoPropertyValue(xPageStyle, "FooterIsOn",
                        False)

                self.xTextDocument.unlockControllers()
            except Exception, exception:
                traceback.print_exc()

    def hasElement(self, sElement):
        if self.xTextDocument is not None:
            mySectionHandler = TextSectionHandler(self.xMSF,
                self.xTextDocument)
            return mySectionHandler.hasTextSectionByName(sElement)
        else:
            return False

    def switchUserField(self, sFieldName, sNewContent, bState):
        myFieldHandler = TextFieldHandler(self.xMSF, self.xTextDocument)
        if bState:
            myFieldHandler.changeUserFieldContent(sFieldName, sNewContent)
        else:
            myFieldHandler.changeUserFieldContent(sFieldName, "")

    def fillSenderWithUserData(self):
        try:
            myFieldHandler = TextFieldHandler(self.xTextDocument,
                self.xTextDocument)
            oUserDataAccess = Configuration.getConfigurationRoot(
                self.xMSF, "org.openoffice.UserProfile/Data", False)
            myFieldHandler.changeUserFieldContent("Company",
                Helper.getUnoObjectbyName(oUserDataAccess, "o"))
            myFieldHandler.changeUserFieldContent("Street",
                Helper.getUnoObjectbyName(oUserDataAccess, "street"))
            myFieldHandler.changeUserFieldContent("PostCode",
                Helper.getUnoObjectbyName(oUserDataAccess, "postalcode"))
            myFieldHandler.changeUserFieldContent(
                PropertyNames.PROPERTY_STATE,
                Helper.getUnoObjectbyName(oUserDataAccess, "st"))
            myFieldHandler.changeUserFieldContent("City",
                Helper.getUnoObjectbyName(oUserDataAccess, "l"))
            myFieldHandler.changeUserFieldContent("Fax",
                Helper.getUnoObjectbyName(oUserDataAccess,
                "facsimiletelephonenumber"))
        except Exception, exception:
            traceback.print_exc()

    def killEmptyUserFields(self):
        myFieldHandler = TextFieldHandler(self.xMSF, self.xTextDocument)
        myFieldHandler.removeUserFieldByContent("")

    def killEmptyFrames(self):
        try:
            if not self.keepLogoFrame:
                xTF = self.getFrameByName("Company Logo",
                self.xTextDocument)
                if xTF is not None:
                    xTF.dispose()

            if not self.keepTypeFrame:
                xTF = self.getFrameByName("Communication Type",
                self.xTextDocument)
                if xTF is not None:
                    xTF.dispose()

        except Exception, e:
            traceback.print_exc()
