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
from ..text.TextDocument import TextDocument, traceback, Helper, \
    TextFieldHandler, Configuration
from ..text.TextSectionHandler import TextSectionHandler
from ..common.PropertyNames import PropertyNames

from com.sun.star.table import BorderLine
from com.sun.star.text.ControlCharacter import PARAGRAPH_BREAK
from com.sun.star.style.ParagraphAdjust import CENTER
from com.sun.star.text.PageNumberType import CURRENT
from com.sun.star.style.NumberingType import ARABIC
from com.sun.star.text.HoriOrientation import NONE as NONEHORI
from com.sun.star.text.VertOrientation import NONE as NONEVERT
from com.sun.star.text.RelOrientation import PAGE_FRAME
from com.sun.star.text.TextContentAnchorType import AT_PAGE
from com.sun.star.text.SizeType import FIX
from com.sun.star.text.WrapTextMode import THROUGHT
from com.sun.star.awt.FontWeight import BOLD

class LetterDocument(TextDocument):

    TextDocument = None

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
            mySectionHandler = TextSectionHandler(
                self.xMSF, TextDocument.xTextDocument)
            oSection = \
                mySectionHandler.xTextDocument.TextSections.getByName(sElement)
            Helper.setUnoPropertyValue(oSection, "IsVisible", bState)
        except Exception:
            traceback.print_exc()

    def updateDateFields(self):
        FH = TextFieldHandler(
            TextDocument.xTextDocument, TextDocument.xTextDocument)
        FH.updateDateFields()

    def switchFooter(self, sPageStyle, bState, bPageNumber, sText):
        if TextDocument.xTextDocument != None:
            try:
                TextDocument.xTextDocument.lockControllers()
                xNameAccess = TextDocument.xTextDocument.StyleFamilies
                xPageStyleCollection = xNameAccess.getByName("PageStyles")
                xPageStyle = xPageStyleCollection.getByName(sPageStyle)
                if bState:
                    Helper.setUnoPropertyValue(xPageStyle, "FooterIsOn", True)
                    xFooterText = \
                        Helper.getUnoPropertyValue(xPageStyle, "FooterText")
                    xFooterText.String = sText
                    if bPageNumber:
                        #Adding the Page Number
                        myCursor = xFooterText.Text.createTextCursor()
                        myCursor.gotoEnd(False)
                        xFooterText.insertControlCharacter(myCursor,
                            PARAGRAPH_BREAK, False)
                        myCursor.setPropertyValue("ParaAdjust", CENTER )

                        xPageNumberField = \
                            TextDocument.xTextDocument.createInstance(
                                "com.sun.star.text.TextField.PageNumber")
                        xPageNumberField.setPropertyValue("SubType", CURRENT)
                        xPageNumberField.NumberingType = ARABIC
                        xFooterText.insertTextContent(xFooterText.End,
                            xPageNumberField, False)

                else:
                    Helper.setUnoPropertyValue(
                        xPageStyle, "FooterIsOn", False)

                TextDocument.xTextDocument.unlockControllers()
            except Exception:
                traceback.print_exc()

    def hasElement(self, sElement):
        if TextDocument.xTextDocument != None:
            SH = TextSectionHandler(self.xMSF, TextDocument.xTextDocument)
            return SH.hasTextSectionByName(sElement)
        else:
            return False

    def switchUserField(self, sFieldName, sNewContent, bState):
        myFieldHandler = TextFieldHandler(
            self.xMSF, TextDocument.xTextDocument)
        if bState:
            myFieldHandler.changeUserFieldContent(sFieldName, sNewContent)
        else:
            myFieldHandler.changeUserFieldContent(sFieldName, "")

    def fillSenderWithUserData(self):
        try:
            myFieldHandler = TextFieldHandler(
                TextDocument.xTextDocument, TextDocument.xTextDocument)
            oUserDataAccess = Configuration.getConfigurationRoot(
                self.xMSF, "org.openoffice.UserProfile/Data", False)
            myFieldHandler.changeUserFieldContent(
                "Company", Helper.getUnoObjectbyName(oUserDataAccess, "o"))
            myFieldHandler.changeUserFieldContent(
                "Street", Helper.getUnoObjectbyName(oUserDataAccess, "street"))
            myFieldHandler.changeUserFieldContent(
                "PostCode",
                Helper.getUnoObjectbyName(oUserDataAccess, "postalcode"))
            myFieldHandler.changeUserFieldContent(
                "City", Helper.getUnoObjectbyName(oUserDataAccess, "l"))
            myFieldHandler.changeUserFieldContent(
                PropertyNames.PROPERTY_STATE,
                Helper.getUnoObjectbyName(oUserDataAccess, "st"))
        except Exception:
            traceback.print_exc()

    def killEmptyUserFields(self):
        myFieldHandler = TextFieldHandler(
            self.xMSF, TextDocument.xTextDocument)
        myFieldHandler.removeUserFieldByContent("")

    def killEmptyFrames(self):
        try:
            if not self.keepLogoFrame:
                xTF = self.getFrameByName(
                    "Company Logo", TextDocument.xTextDocument)
                if xTF != None:
                    xTF.dispose()

            if not self.keepBendMarksFrame:
                xTF = self.getFrameByName(
                    "Bend Marks", TextDocument.xTextDocument)
                if xTF != None:
                    xTF.dispose()

            if not self.keepLetterSignsFrame:
                xTF = self.getFrameByName(
                    "Letter Signs", TextDocument.xTextDocument)
                if xTF != None:
                    xTF.dispose()

            if not self.keepSenderAddressRepeatedFrame:
                xTF = self.getFrameByName(
                    "Sender Address Repeated", TextDocument.xTextDocument)
                if xTF != None:
                    xTF.dispose()

            if not self.keepAddressFrame:
                xTF = self.getFrameByName(
                    "Sender Address", TextDocument.xTextDocument)
                if xTF != None:
                    xTF.dispose()

        except Exception:
            traceback.print_exc()

class BusinessPaperObject(object):

    def __init__(self, FrameText, Width, Height, XPos, YPos):
        self.iWidth = Width
        self.iHeight = Height
        self.iXPos = XPos
        self.iYPos = YPos
        self.xFrame = None
        try:
            self.xFrame = \
                TextDocument.xTextDocument.createInstance(
                    "com.sun.star.text.TextFrame")
            self.setFramePosition()
            Helper.setUnoPropertyValue(
                self.xFrame,
                "AnchorType", AT_PAGE)
            Helper.setUnoPropertyValue(
                self.xFrame,
                "SizeType", FIX)

            Helper.setUnoPropertyValue(
                self.xFrame,
                "TextWrap", THROUGHT)
            Helper.setUnoPropertyValue(
                self.xFrame,
                "Opaque", True);
            Helper.setUnoPropertyValue(
                self.xFrame,
                "BackColor", 15790320)

            myBorder = BorderLine()
            myBorder.OuterLineWidth = 0
            Helper.setUnoPropertyValue(
                self.xFrame,
                "LeftBorder", myBorder)
            Helper.setUnoPropertyValue(
                self.xFrame,
                "RightBorder", myBorder)
            Helper.setUnoPropertyValue(
                self.xFrame,
                "TopBorder", myBorder)
            Helper.setUnoPropertyValue(
                self.xFrame,
                "BottomBorder", myBorder)
            Helper.setUnoPropertyValue(
                self.xFrame,
                "Print", False)
            xTextCursor = \
                TextDocument.xTextDocument.Text.createTextCursor()
            xTextCursor.gotoEnd(True)
            xText = TextDocument.xTextDocument.Text
            xText.insertTextContent(
                xTextCursor, self.xFrame,
                False)

            xFrameText = self.xFrame.Text
            xFrameCursor = xFrameText.createTextCursor()
            xFrameCursor.setPropertyValue("CharWeight", BOLD)
            xFrameCursor.setPropertyValue("CharColor", 16777215)
            xFrameCursor.setPropertyValue("CharFontName", "Albany")
            xFrameCursor.setPropertyValue("CharHeight", 18)

            xFrameText.insertString(xFrameCursor, FrameText, False)
        except Exception:
            traceback.print_exc()

    def setFramePosition(self):
        try:
            Helper.setUnoPropertyValue(
                self.xFrame,
                "HoriOrient", NONEHORI)
            Helper.setUnoPropertyValue(
                self.xFrame,
                "VertOrient", NONEVERT)
            Helper.setUnoPropertyValue(
                self.xFrame,
                PropertyNames.PROPERTY_HEIGHT, self.iHeight)
            Helper.setUnoPropertyValue(
                self.xFrame,
                PropertyNames.PROPERTY_WIDTH, self.iWidth)
            Helper.setUnoPropertyValue(
                self.xFrame,
                "HoriOrientPosition", self.iXPos)
            Helper.setUnoPropertyValue(
                self.xFrame,
                "VertOrientPosition", self.iYPos)
            Helper.setUnoPropertyValue(
                self.xFrame,
                "HoriOrientRelation", PAGE_FRAME)
            Helper.setUnoPropertyValue(
                self.xFrame,
                "VertOrientRelation", PAGE_FRAME)
        except Exception:
            traceback.print_exc()

    def removeFrame(self):
        if self.xFrame is not None:
            try:
                TextDocument.xTextDocument.Text.removeTextContent(
                    self.xFrame)
            except Exception:
                traceback.print_exc()
