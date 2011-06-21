import traceback

class TextSectionHandler(object):
    '''Creates a new instance of TextSectionHandler'''
    def __init__(self, xMSF, xTextDocument):
        self.xMSFDoc = xMSF
        self.xTextDocument = xTextDocument
        self.xText = xTextDocument.Text

    def removeTextSectionbyName(self, SectionName):
        try:
            xAllTextSections = self.xTextDocument.TextSections
            if xAllTextSections.hasByName(SectionName):
                oTextSection = self.xTextDocument.TextSections.getByName(
                    SectionName)
                self.removeTextSection(oTextSection)


        except Exception, exception:
            traceback.print_exc()

    def hasTextSectionByName(self, SectionName):
        xAllTextSections = self.xTextDocument.TextSections
        return xAllTextSections.hasByName(SectionName)

    def removeLastTextSection(self):
        try:
            xAllTextSections = self.xTextDocument.TextSections
            oTextSection = xAllTextSections.getByIndex(
                xAllTextSections.getCount() - 1)
            self.removeTextSection(oTextSection)
        except Exception, exception:
            traceback.print_exc()

    def removeTextSection(self, _oTextSection):
        try:
            self.xText.removeTextContent(_oTextSection)
        except Exception, exception:
            traceback.print_exc()

    def removeInvisibleTextSections(self):
        try:
            xAllTextSections = self.xTextDocument.TextSections
            TextSectionCount = xAllTextSections.getCount()
            i = TextSectionCount - 1
            while i >= 0:
                xTextContentTextSection = xAllTextSections.getByIndex(i)
                if not bool(xTextContentTextSection.getPropertyValue("IsVisible")):
                    self.xText.removeTextContent(xTextContentTextSection)

                i -= 1
        except Exception, exception:
            traceback.print_exc()

    def removeAllTextSections(self):
        try:
            TextSectionCount = self.xTextDocument.TextSections.getCount()
            i = TextSectionCount - 1
            while i >= 0:
                xTextContentTextSection = xAllTextSections.getByIndex(i)
                self.xText.removeTextContent(xTextContentTextSection)
                i -= 1
        except Exception, exception:
            traceback.print_exc()

    def breakLinkofTextSections(self):
        try:
            iSectionCount = self.xTextDocument.TextSections.getCount()
            oSectionLink = SectionFileLink.SectionFileLink()
            oSectionLink.FileURL = ""
            i = 0
            while i < iSectionCount:
                oTextSection = xAllTextSections.getByIndex(i)
                Helper.setUnoPropertyValues(
                    oTextSection, ["FileLink", "LinkRegion"],
                    [oSectionLink, ""])
                i += 1
        except Exception, exception:
            traceback.print_exc()

    def breakLinkOfTextSection(self, oTextSection):
        oSectionLink = SectionFileLink.SectionFileLink()
        oSectionLink.FileURL = ""
        Helper.setUnoPropertyValues(
            oTextSection, ["FileLink", "LinkRegion"],[oSectionLink, ""])

    def linkSectiontoTemplate(self, TemplateName, SectionName):
        try:
            oTextSection = self.xTextDocument.TextSections.getByName(
                SectionName)
            linkSectiontoTemplate(oTextSection, TemplateName, SectionName)
        except Exception, e:
            traceback.print_exc()

    def linkSectiontoTemplate(self, oTextSection, TemplateName, SectionName):
        oSectionLink = SectionFileLink.SectionFileLink()
        oSectionLink.FileURL = TemplateName
        Helper.setUnoPropertyValues(
            oTextSection, ["FileLink", "LinkRegion"],
            [oSectionLink, SectionName])
        NewSectionName = oTextSection.getName()
        if NewSectionName.compareTo(SectionName) != 0:
            oTextSection.setName(SectionName)

    def insertTextSection(self, GroupName, TemplateName, _bAddParagraph):
        try:
            if _bAddParagraph:
                xTextCursor = self.xText.createTextCursor()
                self.xText.insertControlCharacter(
                    xTextCursor, ControlCharacter.PARAGRAPH_BREAK, False)
                xTextCursor.collapseToEnd()

            xSecondTextCursor = self.xText.createTextCursor()
            xSecondTextCursor.gotoEnd(False)
            insertTextSection(GroupName, TemplateName, xSecondTextCursor)
        except IllegalArgumentException, e:
            traceback.print_exc()

    def insertTextSection(self, sectionName, templateName, position):
        try:
            if self.xTextDocument.TextSections.hasByName(sectionName):
                xTextSection = \
                    self.xTextDocument.TextSections.getByName(sectionName)
            else:
                xTextSection = self.xMSFDoc.createInstance(
                    "com.sun.star.text.TextSection")
                position.getText().insertTextContent(
                    position, xTextSection, False)

            linkSectiontoTemplate(xTextSection, templateName, sectionName)
        except Exception, exception:
            traceback.print_exc()

