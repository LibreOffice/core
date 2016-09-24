#! /usr/bin/env python
# -*- Mode: python; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
import math
import unittest
from org.libreoffice.unotest import UnoInProcess
from com.sun.star.container import NoSuchElementException
from com.sun.star.beans import UnknownPropertyException
from com.sun.star.lang import IndexOutOfBoundsException
from com.sun.star.lang import IllegalArgumentException


class CheckStyle(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls._uno = UnoInProcess()
        cls._uno.setUp()

    @classmethod
    def tearDownClass(cls):
        cls._uno.tearDown()

    def test_StyleFamilies(self):
        xDoc = CheckStyle._uno.openEmptyWriterDoc()
        xStyleFamilies = xDoc.StyleFamilies
        self.assertEqual(xStyleFamilies.ImplementationName, "SwXStyleFamilies")
        self.assertEqual(len(xStyleFamilies.SupportedServiceNames), 1)

        for servicename in xStyleFamilies.SupportedServiceNames:
            self.assertIn(servicename, ["com.sun.star.style.StyleFamilies"])
            self.assertTrue(xStyleFamilies.supportsService(servicename))
        self.assertFalse(xStyleFamilies.supportsService("foobarbaz"))
        self.assertTrue(xStyleFamilies.hasElements())
        self.assertRegex(str(xStyleFamilies.ElementType), "com\.sun\.star\.container\.XNameContainer")
        self.assertEqual(len(xStyleFamilies.ElementNames), 7)

        for sFamilyname in xStyleFamilies.ElementNames:
            self.assertIn(sFamilyname,
                          ["CharacterStyles", "ParagraphStyles", "PageStyles", "FrameStyles", "NumberingStyles", "TableStyles", "CellStyles"])

        with self.assertRaises(NoSuchElementException):
            xStyleFamilies.getByName("foobarbaz")
        xDoc.dispose()

    def __test_StyleFamily(self, xFamily, vExpectedNames, sElementImplName):
        self.assertEqual(xFamily.ImplementationName, "XStyleFamily")
        self.assertEqual(len(xFamily.SupportedServiceNames), 1)

        for sServicename in xFamily.SupportedServiceNames:
            self.assertIn(sServicename, ["com.sun.star.style.StyleFamily"])
            self.assertTrue(xFamily.supportsService(sServicename))
        self.assertFalse(xFamily.supportsService("foobarbaz"))
        self.assertTrue(xFamily.hasElements())
        self.assertRegex(str(xFamily.ElementType), "com\.sun\.star\.style\.XStyle")

        with self.assertRaises(NoSuchElementException):
            xFamily.getByName("foobarbaz")

        with self.assertRaises(IndexOutOfBoundsException):
            xFamily.getByIndex(-1)

        for sStylename in xFamily.ElementNames:
            self.assertTrue(xFamily.hasByName(sStylename))
            self.assertEqual(xFamily[sStylename].ImplementationName, sElementImplName)
            self.assertFalse(xFamily[sStylename].isUserDefined())

        vExpectedNames.sort()
        vNames = list(xFamily.ElementNames)
        vNames.sort()
        self.assertListEqual(vNames, vExpectedNames)

    def __test_StyleFamilyIndex(self, xFamily, vExpectedNames, sElementImplName):
        self.assertEqual(xFamily.Count, len(vExpectedNames))

        for nIndex in range(xFamily.Count):
            xStyle = xFamily.getByIndex(nIndex)
            self.assertEqual(xStyle.ImplementationName, sElementImplName)
            self.assertIn(xStyle.Name, vExpectedNames)
            self.assertFalse(xStyle.isUserDefined())

    def __test_StyleFamilyInsert(self, xDoc, xFamily, vExpectedNames, sRightStyle, sWrongStyle):
        xRightStyle = xDoc.createInstance(sRightStyle)
        xRightStyle.Name = "RightStyleOld"
        xWrongStyle = xDoc.createInstance(sWrongStyle)
        xWrongStyle.Name = "WrongtStyleOld"
        xFamily.insertByName("RightStyle", xRightStyle)
        self.assertEqual(xRightStyle.Name, "RightStyle")
        self.assertTrue(xRightStyle.isUserDefined())
        self.assertEqual(xFamily[xRightStyle.Name], xRightStyle)
        self.assertNotEqual(xFamily[xRightStyle.Name], xWrongStyle)
        self.assertTrue(xFamily[xRightStyle.Name].isUserDefined())
        xRightStyle2 = xDoc.createInstance(sRightStyle)
        xRightStyle2.Name = "RightStyle2Old"
        xFamily.replaceByName("RightStyle", xRightStyle2)
        self.assertEqual(xRightStyle2.Name, "RightStyle")
        self.assertEqual(xFamily[xRightStyle2.Name], xRightStyle2)
        xFamily.removeByName(xRightStyle2.Name)

        with self.assertRaises(NoSuchElementException):
            nope = xFamily.getByName("RightStyleOld")
        with self.assertRaises(NoSuchElementException):
            nope = xFamily.getByName("RightStyle")

        with self.assertRaises(NoSuchElementException):
            nope = xFamily.getByName("RightStyle2Old")

        with self.assertRaises(NoSuchElementException):
            nope = xFamily.getByName("RightStyle2")

        with self.assertRaises(IllegalArgumentException):
            xFamily.insertByName("WrongStyle", xWrongStyle)

        with self.assertRaises(NoSuchElementException):
            nope = xFamily.getByName("WrongStyle")

    def test_CharacterFamily(self):
        xDoc = CheckStyle._uno.openEmptyWriterDoc()
        xCharStyles = xDoc.StyleFamilies["CharacterStyles"]
        vEmptyDocStyles = ['Default Style', 'Footnote Symbol', 'Page Number', 'Caption characters', 'Drop Caps', 'Numbering Symbols', 'Bullet Symbols', 'Internet link', 'Visited Internet Link', 'Placeholder', 'Index Link', 'Endnote Symbol', 'Line numbering', 'Main index entry', 'Footnote anchor', 'Endnote anchor', 'Rubies', 'Vertical Numbering Symbols', 'Emphasis', 'Citation', 'Strong Emphasis', 'Source Text', 'Example', 'User Entry', 'Variable', 'Definition', 'Teletype']
        self.__test_StyleFamily(xCharStyles, vEmptyDocStyles, "SwXStyle")
        self.__test_StyleFamilyIndex(xCharStyles, vEmptyDocStyles, "SwXStyle")
        self.__test_StyleFamilyInsert(xDoc, xCharStyles, vEmptyDocStyles, "com.sun.star.style.CharacterStyle", "com.sun.star.style.ParagraphStyle")
        xDoc.dispose()

    def test_ParagraphFamily(self):
        xDoc = CheckStyle._uno.openEmptyWriterDoc()
        xParaStyles = xDoc.StyleFamilies["ParagraphStyles"]
        vEmptyDocStyles = ['Standard', 'Heading', 'Text body', 'List', 'Caption', 'Index', 'First line indent', 'Hanging indent', 'Text body indent', 'Salutation', 'Signature', 'List Indent', 'Marginalia', 'Heading 1', 'Heading 2', 'Heading 3', 'Heading 4', 'Heading 5', 'Heading 6', 'Heading 7', 'Heading 8', 'Heading 9', 'Heading 10', 'Title', 'Subtitle', 'Numbering 1 Start', 'Numbering 1', 'Numbering 1 End', 'Numbering 1 Cont.', 'Numbering 2 Start', 'Numbering 2', 'Numbering 2 End', 'Numbering 2 Cont.', 'Numbering 3 Start', 'Numbering 3', 'Numbering 3 End', 'Numbering 3 Cont.', 'Numbering 4 Start', 'Numbering 4', 'Numbering 4 End', 'Numbering 4 Cont.', 'Numbering 5 Start', 'Numbering 5', 'Numbering 5 End', 'Numbering 5 Cont.', 'List 1 Start', 'List 1', 'List 1 End', 'List 1 Cont.', 'List 2 Start', 'List 2', 'List 2 End', 'List 2 Cont.', 'List 3 Start', 'List 3', 'List 3 End', 'List 3 Cont.', 'List 4 Start', 'List 4', 'List 4 End', 'List 4 Cont.', 'List 5 Start', 'List 5', 'List 5 End', 'List 5 Cont.', 'Index Heading', 'Index 1', 'Index 2', 'Index 3', 'Index Separator', 'Contents Heading', 'Contents 1', 'Contents 2', 'Contents 3', 'Contents 4', 'Contents 5', 'User Index Heading', 'User Index 1', 'User Index 2', 'User Index 3', 'User Index 4', 'User Index 5', 'Contents 6', 'Contents 7', 'Contents 8', 'Contents 9', 'Contents 10', 'Illustration Index Heading', 'Illustration Index 1', 'Object index heading', 'Object index 1', 'Table index heading', 'Table index 1', 'Bibliography Heading', 'Bibliography 1', 'User Index 6', 'User Index 7', 'User Index 8', 'User Index 9', 'User Index 10', 'Header', 'Header left', 'Header right', 'Footer', 'Footer left', 'Footer right', 'Table Contents', 'Table Heading', 'Illustration', 'Table', 'Text', 'Frame contents', 'Footnote', 'Addressee', 'Sender', 'Endnote', 'Drawing', 'Quotations', 'Preformatted Text', 'Horizontal Line', 'List Contents', 'List Heading']
        self.__test_StyleFamily(xParaStyles, vEmptyDocStyles, "SwXStyle")
        self.__test_StyleFamilyIndex(xParaStyles, vEmptyDocStyles, "SwXStyle")
        self.__test_StyleFamilyInsert(xDoc, xParaStyles, vEmptyDocStyles, "com.sun.star.style.ParagraphStyle", "com.sun.star.style.CharacterStyle")
        xDoc.dispose()

    def test_PageFamily(self):
        xDoc = CheckStyle._uno.openEmptyWriterDoc()
        xPageStyles = xDoc.StyleFamilies["PageStyles"]
        vEmptyDocStyles = ['Standard', 'First Page', 'Left Page', 'Right Page', 'Envelope', 'Index', 'HTML', 'Footnote', 'Endnote', 'Landscape']
        self.__test_StyleFamily(xPageStyles, vEmptyDocStyles, "SwXStyle")
        self.__test_StyleFamilyIndex(xPageStyles, vEmptyDocStyles, "SwXStyle")
        self.__test_StyleFamilyInsert(xDoc, xPageStyles, vEmptyDocStyles, "com.sun.star.style.PageStyle", "com.sun.star.style.CharacterStyle")
        xDoc.dispose()

    def test_FrameFamily(self):
        xDoc = CheckStyle._uno.openEmptyWriterDoc()
        xFrameStyles = xDoc.StyleFamilies["FrameStyles"]
        vEmptyDocStyles = ['Formula', 'Frame', 'Graphics', 'Labels', 'Marginalia', 'OLE', 'Watermark']
        self.__test_StyleFamily(xFrameStyles, vEmptyDocStyles, "SwXStyle")
        self.__test_StyleFamilyIndex(xFrameStyles, vEmptyDocStyles, "SwXStyle")
        self.__test_StyleFamilyInsert(xDoc, xFrameStyles, vEmptyDocStyles, "com.sun.star.style.FrameStyle", "com.sun.star.style.CharacterStyle")
        xDoc.dispose()

    def test_NumberingFamily(self):
        xDoc = CheckStyle._uno.openEmptyWriterDoc()
        xNumberingStyles = xDoc.StyleFamilies["NumberingStyles"]
        vEmptyDocStyles = ['List 1', 'List 2', 'List 3', 'List 4', 'List 5', 'Numbering 1', 'Numbering 2', 'Numbering 3', 'Numbering 4', 'Numbering 5']
        self.__test_StyleFamily(xNumberingStyles, vEmptyDocStyles, "SwXStyle")
        self.__test_StyleFamilyIndex(xNumberingStyles, vEmptyDocStyles, "SwXStyle")
        self.__test_StyleFamilyInsert(xDoc, xNumberingStyles, vEmptyDocStyles, "com.sun.star.style.NumberingStyle", "com.sun.star.style.CharacterStyle")
        xDoc.dispose()

    def test_TableFamily(self):
        xDoc = CheckStyle._uno.openEmptyWriterDoc()
        xTableStyles = xDoc.StyleFamilies["TableStyles"]
        vEmptyDocStyles = ['Default Style']
        self.__test_StyleFamily(xTableStyles, vEmptyDocStyles, "SwXTextTableStyle")
        self.__test_StyleFamilyIndex(xTableStyles, vEmptyDocStyles, "SwXTextTableStyle")
        self.__test_StyleFamilyInsert(xDoc, xTableStyles, vEmptyDocStyles, "com.sun.star.style.TableStyle", "com.sun.star.style.CharacterStyle")
        for sStyleName in vEmptyDocStyles:
            self.assertIsNotNone(xTableStyles.getByName(sStyleName))
        #check SwXTextCellStyles
        vCellStyles = ["first-row", "last-row", "first-column", "last-column", "body", "even-rows", "odd-rows", "even-columns", "odd-columns", "background"]
        xDefaultTableStyle = xTableStyles.getByIndex(0)
        for sCellStyle in vCellStyles:
            xCellStyle = xDefaultTableStyle.getByName(sCellStyle)
            self.assertIsNotNone(xCellStyle.getPropertyValue("BackColor"))
            with self.assertRaises(UnknownPropertyException):
                xCellStyle.getPropertyValue("foobarbaz")
        xDoc.dispose()

    def test_tableStyleIsInUse(self):
        xDoc = CheckStyle._uno.openEmptyWriterDoc()
        xBodyText = xDoc.getText()
        xCursor = xBodyText.createTextCursor()
        xTable = xDoc.createInstance("com.sun.star.text.TextTable")
        xTable.initialize(1, 1)
        xBodyText.insertTextContent(xCursor, xTable, True)
        xDefaultTableStyle = xDoc.StyleFamilies.getByName("TableStyles").getByName("Default Style")
        xDefaultCellStyle = xDoc.StyleFamilies.getByName("CellStyles").getByName("Default Style.1")
        self.assertFalse(xDefaultTableStyle.isInUse())
        self.assertFalse(xDefaultCellStyle.isInUse())
        xTable.setPropertyValue("TableTemplateName", "Default Style")
        self.assertTrue(xDefaultTableStyle.isInUse())
        self.assertTrue(xDefaultCellStyle.isInUse())
        xTable.setPropertyValue("TableTemplateName", "")
        self.assertFalse(xDefaultTableStyle.isInUse())
        self.assertFalse(xDefaultCellStyle.isInUse())
        xTableStyle = xDoc.createInstance("com.sun.star.style.TableStyle")
        self.assertFalse(xTableStyle.isInUse())
        xDoc.StyleFamilies.getByName("TableStyles").insertByName("Test Table Style", xTableStyle)
        self.assertFalse(xTableStyle.isInUse())
        xTable.setPropertyValue("TableTemplateName", "Test Table Style")
        self.assertTrue(xTableStyle.isInUse())
        xTable.setPropertyValue("TableTemplateName", "")
        self.assertFalse(xTableStyle.isInUse())
        xDoc.dispose()

    def test_CellFamily(self):
        xDoc = CheckStyle._uno.openEmptyWriterDoc()
        xCellStyles = xDoc.StyleFamilies["CellStyles"]
        vEmptyDocStyles = ['Default Style.1', 'Default Style.2', 'Default Style.3', 'Default Style.4', 'Default Style.5', 'Default Style.6', 'Default Style.7', 'Default Style.8', 'Default Style.9', 'Default Style.10', 'Default Style.11', 'Default Style.12', 'Default Style.13', 'Default Style.14', 'Default Style.15', 'Default Style.16']
        self.__test_StyleFamily(xCellStyles, vEmptyDocStyles, "SwXTextCellStyle")
        self.__test_StyleFamilyIndex(xCellStyles, vEmptyDocStyles, "SwXTextCellStyle")
        self.__test_StyleFamilyInsert(xDoc, xCellStyles, vEmptyDocStyles, "com.sun.star.style.CellStyle", "com.sun.star.style.CharacterStyle")
        xTableStyle = xDoc.createInstance("com.sun.star.style.TableStyle")
        xCellStyle = xTableStyle.getByName("first-row")
        xDoc.StyleFamilies.getByName("TableStyles").insertByName("Test Table Style", xTableStyle)
        self.assertEqual(xTableStyle.getByName("first-row"), xCellStyle)
        self.assertEqual(xTableStyle.getByName("first-row"), xDoc.StyleFamilies.getByName("TableStyles").getByName("Test Table Style").getByName("first-row"))
        #replaceByName
        with self.assertRaises(NoSuchElementException):
            xTableStyle.replaceByName("foobarbaz", xCellStyle)
        xCellStyle = xDoc.createInstance("com.sun.star.style.CellStyle")
        with self.assertRaises(IllegalArgumentException):       #replace with not inserted cell style
            xTableStyle.replaceByName("first-row", xCellStyle)
        xTableStyle2 = xDoc.createInstance("com.sun.star.style.TableStyle")
        with self.assertRaises(IllegalArgumentException):       #replace with other family style
            xTableStyle.replaceByName("first-row", xTableStyle2)
        #replace with already assigned cell style
        xTableStyle.replaceByName("first-row", xTableStyle2.getByName("first-row"))
        self.assertEqual(xTableStyle.getByName("first-row"), xTableStyle2.getByName("first-row"))
        xDoc.StyleFamilies.getByName("CellStyles").insertByName("Test Cell Style", xCellStyle)
        xTableStyle.replaceByName("first-row", xCellStyle)
        self.assertEqual(xTableStyle.getByName("first-row"), xCellStyle)
        xDoc.dispose()

if __name__ == '__main__':
    unittest.main()

# /* vim:set shiftwidth=4 softtabstop=4 expandtab: */
