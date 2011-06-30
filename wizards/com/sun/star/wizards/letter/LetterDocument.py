from text.TextDocument import *
from text.TextSectionHandler import TextSectionHandler
from common.PropertyNames import PropertyNames

from com.sun.star.text.ControlCharacter import PARAGRAPH_BREAK
from com.sun.star.style.ParagraphAdjust import CENTER
from com.sun.star.text.PageNumberType import CURRENT
from com.sun.star.style.NumberingType import ARABIC

class LetterDocument(TextDocument):

    def __init__(self, xMSF, listener):
        super(LetterDocument,self).__init__(xMSF, listener, None,
            "WIZARD_LIVE_PREVIEW")
        self.keepLogoFrame = True
        self.keepBendMarksFrame = True
        self.keepLetterSignsFrame = True
        self.keepSenderAddressRepeatedFrame = True
        self.keepAddressFrame = True

    def switchElement(self, sElement, bState):
        try:
            mySectionHandler = TextSectionHandler(self.xMSF, self.xTextDocument)
            oSection = mySectionHandler.xTextDocument.TextSections.getByName(sElement)
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
                xNameAccess = self.xTextDocument.StyleFamilies
                xPageStyleCollection = xNameAccess.getByName("PageStyles")
                xPageStyle = xPageStyleCollection.getByName(sPageStyle)
                if bState:
                    Helper.setUnoPropertyValue(xPageStyle, "FooterIsOn",True)
                    xFooterText = Helper.getUnoPropertyValue(xPageStyle, "FooterText")
                    xFooterText.String = sText
                    if bPageNumber:
                        #Adding the Page Number
                        myCursor = xFooterText.createTextCursor()
                        myCursor.gotoEnd(False)
                        xFooterText.insertControlCharacter(myCursor, PARAGRAPH_BREAK, False)
                        myCursor.setPropertyValue("ParaAdjust", CENTER)
                        xPageNumberField = self.xTextDocument.createInstance("com.sun.star.text.TextField.PageNumber")
                        xPageNumberField.setPropertyValue("SubType", CURRENT)
                        xPageNumberField.setPropertyValue("NumberingType", ARABIC)
                        xFooterText.insertTextContent(xFooterText.getEnd(), xPageNumberField, False)
                else:
                    Helper.setUnoPropertyValue(xPageStyle, "FooterIsOn", False)

                self.xTextDocument.unlockControllers()
            except Exception, exception:
                traceback.print_exc()

    def hasElement(self, sElement):
        if self.xTextDocument is not None:
            SH = TextSectionHandler(self.xMSF, self.xTextDocument)
            return SH.hasTextSectionByName(sElement)
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
            myFieldHandler = TextFieldHandler(self.xMSF, self.xTextDocument)
            oUserDataAccess = Configuration.getConfigurationRoot(self.xMSF, "org.openoffice.UserProfile/Data", False)
            myFieldHandler.changeUserFieldContent("Company", Helper.getUnoObjectbyName(oUserDataAccess, "o"))
            myFieldHandler.changeUserFieldContent("Street", Helper.getUnoObjectbyName(oUserDataAccess, "street"))
            myFieldHandler.changeUserFieldContent("PostCode", Helper.getUnoObjectbyName(oUserDataAccess, "postalcode"))
            myFieldHandler.changeUserFieldContent("City", Helper.getUnoObjectbyName(oUserDataAccess, "l"))
            myFieldHandler.changeUserFieldContent(PropertyNames.PROPERTY_STATE, Helper.getUnoObjectbyName(oUserDataAccess, "st"))
        except Exception, exception:
            traceback.print_exc()

    def killEmptyUserFields(self):
        myFieldHandler = TextFieldHandle(self.xMSF, self.xTextDocument)
        myFieldHandler.removeUserFieldByContent("")

    def killEmptyFrames(self):
        try:
            if not self.keepLogoFrame:
                xTF = TextFrameHandler.getFrameByName("Company Logo", self.xTextDocument)
            if xTF is not None:
                xTF.dispose()

            if not self.keepBendMarksFrame:
                xTF = TextFrameHandler.getFrameByName("Bend Marks", self.xTextDocument)
            if xTF is not None:
                xTF.dispose()

            if not self.keepLetterSignsFrame:
                xTF = TextFrameHandler.getFrameByName("Letter Signs", self.xTextDocument)
            if xTF is not None:
                xTF.dispose()

            if not self.keepSenderAddressRepeatedFrame:
                xTF = TextFrameHandler.getFrameByName("Sender Address Repeated", self.xTextDocument)
            if xTF is not None:
                xTF.dispose()

            if not self.keepAddressFrame:
                xTF = TextFrameHandler.getFrameByName("Sender Address", self.xTextDocument)
            if xTF is not None:
                xTF.dispose()

        except Exception, e:
            traceback.print_exc()

class BusinessPaperObject(object):

    def __init__(self, FrameText, Width, Height, XPos, YPos):
        self.iWidth = Width
        self.iHeight = Height
        self.iXPos = XPos
        self.iYPos = YPos

        try:
            xFrame = self.xTextDocument.createInstance("com.sun.star.text.TextFrame")
            self.setFramePosition()
            Helper.setUnoPropertyValue(xFrame, "AnchorType", TextContentAnchorType.AT_PAGE)
            Helper.setUnoPropertyValue(xFrame, "SizeType", SizeType.FIX)

            Helper.setUnoPropertyValue(xFrame, "TextWrap", WrapTextMode.THROUGHT)
            Helper.setUnoPropertyValue(xFrame, "Opaque", Boolean.TRUE)
            Helper.setUnoPropertyValue(xFrame, "BackColor", 15790320)

            myBorder = BorderLine()
            myBorder.OuterLineWidth = 0
            Helper.setUnoPropertyValue(xFrame, "LeftBorder", myBorder)
            Helper.setUnoPropertyValue(xFrame, "RightBorder", myBorder)
            Helper.setUnoPropertyValue(xFrame, "TopBorder", myBorder)
            Helper.setUnoPropertyValue(xFrame, "BottomBorder", myBorder)
            Helper.setUnoPropertyValue(xFrame, "Print", False)

            xTextCursor = self.xTextDocument.Text.createTextCursor()
            xTextCursor.gotoEnd(True)
            xText = self.xTextDocument.Text
            xText.insertTextContent(xTextCursor, xFrame, False)

            xFrameText = xFrame.Text
            xFrameCursor = xFrameText.createTextCursor()
            xFrameCursor.setPropertyValue("CharWeight", com.sun.star.awt.FontWeight.BOLD)
            xFrameCursor.setPropertyValue("CharColor", 16777215)
            xFrameCursor.setPropertyValue("CharFontName", "Albany")
            xFrameCursor.setPropertyValue("CharHeight", 18)

            xFrameText.insertString(xFrameCursor, FrameText, False)

        except Exception:
            traceback.print_exc()

    def setFramePosition(self):
        Helper.setUnoPropertyValue(xFrame, "HoriOrient", HoriOrientation.NONE)
        Helper.setUnoPropertyValue(xFrame, "VertOrient", VertOrientation.NONE)
        Helper.setUnoPropertyValue(xFrame, PropertyNames.PROPERTY_HEIGHT, iHeight)
        Helper.setUnoPropertyValue(xFrame, PropertyNames.PROPERTY_WIDTH, iWidth)
        Helper.setUnoPropertyValue(xFrame, "HoriOrientPosition", iXPos)
        Helper.setUnoPropertyValue(xFrame, "VertOrientPosition", iYPos)
        Helper.setUnoPropertyValue(xFrame, "HoriOrientRelation", RelOrientation.PAGE_FRAME)
        Helper.setUnoPropertyValue(xFrame, "VertOrientRelation", RelOrientation.PAGE_FRAME)

    def removeFrame(self):
        if xFrame is not None:
            try:
                self.xTextDocument.Text.removeTextContent(xFrame)
            except Exception:
                traceback.print_exc()

