import traceback

class TextSectionHandler(object):
    '''Creates a new instance of TextSectionHandler'''
    def __init__(self, xMSF, xTextDocument):
        self.xMSFDoc = xMSF
        self.xTextDocument = xTextDocument
        self.xText = xTextDocument.getText()

    def removeTextSectionbyName(self, SectionName):
        try:
            xAllTextSections = self.xTextDocument.getTextSections()
            if xAllTextSections.hasByName(SectionName) == True:
                oTextSection = self.xTextDocument.getTextSections().getByName(SectionName)
                removeTextSection(oTextSection)

        except Exception, exception:
            traceback.print_exc()

    def hasTextSectionByName(self, SectionName):
        xAllTextSections = self.xTextDocument.getTextSections()
        return xAllTextSections.hasByName(SectionName)

    def removeLastTextSection(self):
        try:
            xAllTextSections = self.xTextDocument.getTextSections()
            oTextSection = xAllTextSections.getByIndex(xAllTextSections.getCount() - 1)
            removeTextSection(oTextSection)
        except Exception, exception:
            traceback.print_exc()

    def removeTextSection(self, _oTextSection):
        try:
            self.xText.removeTextContent(_oTextSection)
        except Exception, exception:
            traceback.print_exc()

    def removeInvisibleTextSections(self):
        try:
            xAllTextSections = self.xTextDocument.getTextSections()
            TextSectionCount = xAllTextSections.getCount()
            i = TextSectionCount - 1
            while i >= 0:
                xTextContentTextSection = xAllTextSections.getByIndex(i)
                bRemoveTextSection = (not AnyConverter.toBoolean(xTextContentTextSection.getPropertyValue("IsVisible")))
                if bRemoveTextSection:
                    self.xText.removeTextContent(xTextContentTextSection)

                i -= 1
        except Exception, exception:
            traceback.print_exc()

    def removeAllTextSections(self):
        try:
            TextSectionCount = self.xTextDocument.getTextSections().getCount()
            i = TextSectionCount - 1
            while i >= 0:
                xTextContentTextSection = xAllTextSections.getByIndex(i)
                self.xText.removeTextContent(xTextContentTextSection)
                i -= 1
        except Exception, exception:
            traceback.print_exc()

    def breakLinkofTextSections(self):
        try:
            iSectionCount = self.xTextDocument.getTextSections().getCount()
            oSectionLink = SectionFileLink.SectionFileLink()
            oSectionLink.FileURL = ""
            i = 0
            while i < iSectionCount:
                oTextSection = xAllTextSections.getByIndex(i)
                Helper.setUnoPropertyValues(oTextSection, ["FileLink", "LinkRegion"], [oSectionLink, ""])
                i += 1
        except Exception, exception:
            traceback.print_exc()

    def breakLinkOfTextSection(self, oTextSection):
        oSectionLink = SectionFileLink.SectionFileLink()
        oSectionLink.FileURL = ""
        Helper.setUnoPropertyValues(oTextSection, ["FileLink", "LinkRegion"],[oSectionLink, ""])

    def linkSectiontoTemplate(self, TemplateName, SectionName):
        try:
            oTextSection = self.xTextDocument.getTextSections().getByName(SectionName)
            linkSectiontoTemplate(oTextSection, TemplateName, SectionName)
        except Exception, e:
            traceback.print_exc()

    def linkSectiontoTemplate(self, oTextSection, TemplateName, SectionName):
        oSectionLink = SectionFileLink.SectionFileLink()
        oSectionLink.FileURL = TemplateName
        Helper.setUnoPropertyValues(oTextSection, ["FileLink", "LinkRegion"],[oSectionLink, SectionName])
        NewSectionName = oTextSection.getName()
        if NewSectionName.compareTo(SectionName) != 0:
            oTextSection.setName(SectionName)

    def insertTextSection(self, GroupName, TemplateName, _bAddParagraph):
        try:
            if _bAddParagraph:
                xTextCursor = self.xText.createTextCursor()
                self.xText.insertControlCharacter(xTextCursor, ControlCharacter.PARAGRAPH_BREAK, False)
                xTextCursor.collapseToEnd()

            xSecondTextCursor = self.xText.createTextCursor()
            xSecondTextCursor.gotoEnd(False)
            insertTextSection(GroupName, TemplateName, xSecondTextCursor)
        except IllegalArgumentException, e:
            traceback.print_exc()

    def insertTextSection(self, sectionName, templateName, position):
        try:
            if self.xTextDocument.getTextSections().hasByName(sectionName) == True:
                xTextSection = self.xTextDocument.getTextSections().getByName(sectionName)
            else:
                xTextSection = self.xMSFDoc.createInstance("com.sun.star.text.TextSection")
                position.getText().insertTextContent(position, xTextSection, False)

            linkSectiontoTemplate(xTextSection, templateName, sectionName)
        except Exception, exception:
            traceback.print_exc()

