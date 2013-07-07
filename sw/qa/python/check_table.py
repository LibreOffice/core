import unittest
from org.libreoffice.unotest import UnoInProcess
from com.sun.star.uno import RuntimeException
from com.sun.star.table import BorderLine
from com.sun.star.table import BorderLine2
from com.sun.star.table.BorderLineStyle import (DOUBLE, SOLID, EMBOSSED,\
    THICKTHIN_LARGEGAP, DASHED, DOTTED)

class CheckTable(unittest.TestCase):
    _uno = None
    _xDoc = None
    _xDocF = None
    @classmethod
    def setUpClass(cls):
        cls._uno = UnoInProcess()
        cls._uno.setUp()
        cls._xDoc = cls._uno.openEmptyWriterDoc()
        cls._xDocF = cls._uno.openEmptyWriterDoc()
    @classmethod
    def tearDownClass(cls):
        cls._uno.tearDown()

    def test_tableborder(self):
        xDoc = self.__class__._xDoc
        # insert table
        xTable = xDoc.createInstance("com.sun.star.text.TextTable")
        xTable.initialize(3, 3)
        xText = xDoc.getText()
        xCursor = xText.createTextCursor()
        xText.insertTextContent(xCursor, xTable, False)

        border = xTable.getPropertyValue("TableBorder")

        self.assertTrue(border.IsTopLineValid)
        self.assertEquals(0, border.TopLine.InnerLineWidth)
        self.assertEquals(2, border.TopLine.OuterLineWidth)
        self.assertEquals(0, border.TopLine.LineDistance)
        self.assertEquals(0, border.TopLine.Color)

        self.assertTrue(border.IsBottomLineValid)
        self.assertEquals(0, border.BottomLine.InnerLineWidth)
        self.assertEquals(2, border.BottomLine.OuterLineWidth)
        self.assertEquals(0, border.BottomLine.LineDistance)
        self.assertEquals(0, border.BottomLine.Color)

        self.assertTrue(border.IsLeftLineValid)
        self.assertEquals(0, border.LeftLine.InnerLineWidth)
        self.assertEquals(2, border.LeftLine.OuterLineWidth)
        self.assertEquals(0, border.LeftLine.LineDistance)
        self.assertEquals(0, border.LeftLine.Color)

        self.assertTrue(border.IsRightLineValid)
        self.assertEquals(0, border.RightLine.InnerLineWidth)
        self.assertEquals(2, border.RightLine.OuterLineWidth)
        self.assertEquals(0, border.RightLine.LineDistance)
        self.assertEquals(0, border.RightLine.Color)

        self.assertTrue(border.IsHorizontalLineValid)
        self.assertEquals(0, border.HorizontalLine.InnerLineWidth)
        self.assertEquals(2, border.HorizontalLine.OuterLineWidth)
        self.assertEquals(0, border.HorizontalLine.LineDistance)
        self.assertEquals(0, border.HorizontalLine.Color)

        self.assertTrue(border.IsVerticalLineValid)
        self.assertEquals(0, border.VerticalLine.InnerLineWidth)
        self.assertEquals(2, border.VerticalLine.OuterLineWidth)
        self.assertEquals(0, border.VerticalLine.LineDistance)
        self.assertEquals(0, border.VerticalLine.Color)

        self.assertTrue(border.IsDistanceValid)
        self.assertEquals(97, border.Distance)
    # set border
        border.TopLine        = BorderLine(0,      11, 19, 19)
        border.BottomLine     = BorderLine(0xFF,   00, 11, 00)
        border.HorizontalLine = BorderLine(0xFF00, 00, 90, 00)
        xTable.setPropertyValue("TableBorder", border)
    # read set border
        border = xTable.getPropertyValue("TableBorder")

        self.assertTrue(border.IsTopLineValid)
        self.assertEquals(11, border.TopLine.InnerLineWidth)
        self.assertEquals(19, border.TopLine.OuterLineWidth)
        self.assertEquals(19, border.TopLine.LineDistance)
        self.assertEquals(0, border.TopLine.Color)

        self.assertTrue(border.IsBottomLineValid)
        self.assertEquals(0, border.BottomLine.InnerLineWidth)
        self.assertEquals(11, border.BottomLine.OuterLineWidth)
        self.assertEquals(0, border.BottomLine.LineDistance)
        self.assertEquals(0xFF, border.BottomLine.Color)

        self.assertTrue(border.IsLeftLineValid)
        self.assertEquals(0, border.LeftLine.InnerLineWidth)
        self.assertEquals(2, border.LeftLine.OuterLineWidth)
        self.assertEquals(0, border.LeftLine.LineDistance)
        self.assertEquals(0, border.LeftLine.Color)

        self.assertTrue(border.IsRightLineValid)
        self.assertEquals(0, border.RightLine.InnerLineWidth)
        self.assertEquals(2, border.RightLine.OuterLineWidth)
        self.assertEquals(0, border.RightLine.LineDistance)
        self.assertEquals(0, border.RightLine.Color)

        self.assertTrue(border.IsHorizontalLineValid)
        self.assertEquals(0, border.HorizontalLine.InnerLineWidth)
        self.assertEquals(90, border.HorizontalLine.OuterLineWidth)
        self.assertEquals(0, border.HorizontalLine.LineDistance)
        self.assertEquals(0xFF00, border.HorizontalLine.Color)

        self.assertTrue(border.IsVerticalLineValid)
        self.assertEquals(0, border.VerticalLine.InnerLineWidth)
        self.assertEquals(2, border.VerticalLine.OuterLineWidth)
        self.assertEquals(0, border.VerticalLine.LineDistance)
        self.assertEquals(0, border.VerticalLine.Color)

        self.assertTrue(border.IsDistanceValid)
        self.assertEquals(97, border.Distance)

        border2 = xTable.getPropertyValue("TableBorder2")
        self.assertTrue(border2.IsTopLineValid)
        self.assertEquals(11, border2.TopLine.InnerLineWidth)
        self.assertEquals(19, border2.TopLine.OuterLineWidth)
        self.assertEquals(19,  border2.TopLine.LineDistance)
        self.assertEquals(0, border2.TopLine.Color)
        self.assertEquals(DOUBLE, border2.TopLine.LineStyle)
        self.assertEquals(49, border2.TopLine.LineWidth)

        self.assertTrue(border2.IsBottomLineValid)
        self.assertEquals(0, border2.BottomLine.InnerLineWidth)
        self.assertEquals(11, border2.BottomLine.OuterLineWidth)
        self.assertEquals(0, border2.BottomLine.LineDistance)
        self.assertEquals(0xFF, border2.BottomLine.Color)
        self.assertEquals(SOLID, border2.BottomLine.LineStyle)
        self.assertEquals(11, border2.BottomLine.LineWidth)

        self.assertTrue(border2.IsLeftLineValid)
        self.assertEquals(0, border2.LeftLine.InnerLineWidth)
        self.assertEquals(2, border2.LeftLine.OuterLineWidth)
        self.assertEquals(0, border2.LeftLine.LineDistance)
        self.assertEquals(0, border2.LeftLine.Color)
        self.assertEquals(SOLID, border2.LeftLine.LineStyle)
        self.assertEquals(2, border2.LeftLine.LineWidth)

        self.assertTrue(border2.IsRightLineValid)
        self.assertEquals(0, border2.RightLine.InnerLineWidth)
        self.assertEquals(2, border2.RightLine.OuterLineWidth)
        self.assertEquals(0, border2.RightLine.LineDistance)
        self.assertEquals(0, border2.RightLine.Color)
        self.assertEquals(SOLID, border2.RightLine.LineStyle)
        self.assertEquals(2, border2.RightLine.LineWidth)

        self.assertTrue(border2.IsHorizontalLineValid)
        self.assertEquals(0, border2.HorizontalLine.InnerLineWidth)
        self.assertEquals(90, border2.HorizontalLine.OuterLineWidth)
        self.assertEquals(0, border2.HorizontalLine.LineDistance)
        self.assertEquals(0xFF00, border2.HorizontalLine.Color)
        self.assertEquals(SOLID, border2.HorizontalLine.LineStyle)
        self.assertEquals(90, border2.HorizontalLine.LineWidth)

        self.assertTrue(border2.IsVerticalLineValid)
        self.assertEquals(0, border2.VerticalLine.InnerLineWidth)
        self.assertEquals(2, border2.VerticalLine.OuterLineWidth)
        self.assertEquals(0, border2.VerticalLine.LineDistance)
        self.assertEquals(0, border2.VerticalLine.Color)
        self.assertEquals(SOLID, border2.VerticalLine.LineStyle)
        self.assertEquals(2, border2.VerticalLine.LineWidth)

        self.assertTrue(border2.IsDistanceValid)
        self.assertEquals(97, border2.Distance)
    # set border2
        border2.RightLine      = BorderLine2(0,      0, 0, 0, THICKTHIN_LARGEGAP, 120)
        border2.LeftLine       = BorderLine2(0,      0, 0, 0, EMBOSSED, 90)
        border2.VerticalLine   = BorderLine2(0xFF,   0, 90, 0, DOTTED, 0)
        border2.HorizontalLine = BorderLine2(0xFF00, 0, 0, 0, DASHED, 11)
        xTable.setPropertyValue("TableBorder2", border2)
    # read set border2
        border2 = xTable.getPropertyValue("TableBorder2")

        self.assertTrue(border2.IsTopLineValid)
        self.assertEquals(11, border2.TopLine.InnerLineWidth)
        self.assertEquals(19, border2.TopLine.OuterLineWidth)
        self.assertEquals(19, border2.TopLine.LineDistance)
        self.assertEquals(0, border2.TopLine.Color)
        self.assertEquals(DOUBLE, border2.TopLine.LineStyle)
        self.assertEquals(49, border2.TopLine.LineWidth)

        self.assertTrue(border2.IsBottomLineValid)
        self.assertEquals(0, border2.BottomLine.InnerLineWidth)
        self.assertEquals(11, border2.BottomLine.OuterLineWidth)
        self.assertEquals(0, border2.BottomLine.LineDistance)
        self.assertEquals(0xFF, border2.BottomLine.Color)
        self.assertEquals(SOLID, border2.BottomLine.LineStyle)
        self.assertEquals(11, border2.BottomLine.LineWidth)

        self.assertTrue(border2.IsLeftLineValid)
        self.assertEquals(23, border2.LeftLine.InnerLineWidth)
        self.assertEquals(23, border2.LeftLine.OuterLineWidth)
        self.assertEquals(46, border2.LeftLine.LineDistance)
        self.assertEquals(0, border2.LeftLine.Color)
        self.assertEquals(EMBOSSED, border2.LeftLine.LineStyle)
        self.assertEquals(90, border2.LeftLine.LineWidth)

        self.assertTrue(border2.IsRightLineValid)
        self.assertEquals(53, border2.RightLine.InnerLineWidth)
        self.assertEquals(26, border2.RightLine.OuterLineWidth)
        self.assertEquals(41, border2.RightLine.LineDistance)
        self.assertEquals(0, border2.RightLine.Color)
        self.assertEquals(THICKTHIN_LARGEGAP, border2.RightLine.LineStyle)
        self.assertEquals(120, border2.RightLine.LineWidth)

        self.assertTrue(border2.IsHorizontalLineValid)
        self.assertEquals(0, border2.HorizontalLine.InnerLineWidth)
        self.assertEquals(11, border2.HorizontalLine.OuterLineWidth)
        self.assertEquals(0, border2.HorizontalLine.LineDistance)
        self.assertEquals(0xFF00, border2.HorizontalLine.Color)
        self.assertEquals(DASHED, border2.HorizontalLine.LineStyle)
        self.assertEquals(11, border2.HorizontalLine.LineWidth)

        self.assertTrue(border2.IsVerticalLineValid)
        self.assertEquals(0, border2.VerticalLine.InnerLineWidth)
        self.assertEquals(90, border2.VerticalLine.OuterLineWidth)
        self.assertEquals(0, border2.VerticalLine.LineDistance)
        self.assertEquals(0xFF, border2.VerticalLine.Color)
        self.assertEquals(DOTTED, border2.VerticalLine.LineStyle)
        self.assertEquals(90, border2.VerticalLine.LineWidth)

        self.assertTrue(border2.IsDistanceValid)
        self.assertEquals(97, border2.Distance)

    # close document
        xDoc.dispose()
    # set border

    def test_fdo58242(self):

        xDoc = self.__class__._xDocF
    # insert table
        xTable = xDoc.createInstance("com.sun.star.text.TextTable")
        xTable.initialize(3, 3)
        xText = xDoc.getText()
        xCursor = xText.createTextCursor()
        xText.insertTextContent(xCursor, xTable, False)
    # get anchor
        xAnchor = xTable.getAnchor()

    # check all properties on the anchor - shouldn't crash despite
    # pointing to a non-SwTxtNode
        xPropsInfo = xAnchor.getPropertySetInfo()
        for i in xPropsInfo.getProperties():
            try:
                xAnchor.getPropertyValue(i.Name)
            except RuntimeException:
                pass
    # close document
        xDoc.dispose()

if __name__ == '__main__':
    unittest.main()

# /* vim:set shiftwidth=4 softtabstop=4 expandtab: */
