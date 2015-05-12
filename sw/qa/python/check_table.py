import unittest
from org.libreoffice.unotest import UnoInProcess
from com.sun.star.uno import RuntimeException
from com.sun.star.table import BorderLine
from com.sun.star.table import BorderLine2
from com.sun.star.table.BorderLineStyle import (DOUBLE, SOLID, EMBOSSED,\
    THICKTHIN_LARGEGAP, DASHED, DOTTED)

class CheckTable(unittest.TestCase):
    _uno = None
    def _fill_table(self, xTable):
        for x in range(3):
            for y in range(3):
                xTable.getCellByPosition(x, y).String = 'Cell %d %d' % (x, y)
    def _check_table(self, xTable):
        for x in range(3):
            for y in range(3):
                self.assertEqual('Cell %d %d' % (x, y), xTable.getCellByPosition(x, y).String)
    @classmethod
    def setUpClass(cls):
        cls._uno = UnoInProcess()
        cls._uno.setUp()
    @classmethod
    def tearDownClass(cls):
        cls._uno.tearDown()

    def test_tableborder(self):
        xDoc = CheckTable._uno.openEmptyWriterDoc()
        # insert table
        xTable = xDoc.createInstance("com.sun.star.text.TextTable")
        xTable.initialize(3, 3)
        xText = xDoc.getText()
        xCursor = xText.createTextCursor()
        xText.insertTextContent(xCursor, xTable, False)

        border = xTable.getPropertyValue("TableBorder")

        self.assertTrue(border.IsTopLineValid)
        self.assertEqual(0, border.TopLine.InnerLineWidth)
        self.assertEqual(2, border.TopLine.OuterLineWidth)
        self.assertEqual(0, border.TopLine.LineDistance)
        self.assertEqual(0, border.TopLine.Color)

        self.assertTrue(border.IsBottomLineValid)
        self.assertEqual(0, border.BottomLine.InnerLineWidth)
        self.assertEqual(2, border.BottomLine.OuterLineWidth)
        self.assertEqual(0, border.BottomLine.LineDistance)
        self.assertEqual(0, border.BottomLine.Color)

        self.assertTrue(border.IsLeftLineValid)
        self.assertEqual(0, border.LeftLine.InnerLineWidth)
        self.assertEqual(2, border.LeftLine.OuterLineWidth)
        self.assertEqual(0, border.LeftLine.LineDistance)
        self.assertEqual(0, border.LeftLine.Color)

        self.assertTrue(border.IsRightLineValid)
        self.assertEqual(0, border.RightLine.InnerLineWidth)
        self.assertEqual(2, border.RightLine.OuterLineWidth)
        self.assertEqual(0, border.RightLine.LineDistance)
        self.assertEqual(0, border.RightLine.Color)

        self.assertTrue(border.IsHorizontalLineValid)
        self.assertEqual(0, border.HorizontalLine.InnerLineWidth)
        self.assertEqual(2, border.HorizontalLine.OuterLineWidth)
        self.assertEqual(0, border.HorizontalLine.LineDistance)
        self.assertEqual(0, border.HorizontalLine.Color)

        self.assertTrue(border.IsVerticalLineValid)
        self.assertEqual(0, border.VerticalLine.InnerLineWidth)
        self.assertEqual(2, border.VerticalLine.OuterLineWidth)
        self.assertEqual(0, border.VerticalLine.LineDistance)
        self.assertEqual(0, border.VerticalLine.Color)

        self.assertTrue(border.IsDistanceValid)
        self.assertEqual(97, border.Distance)
    # set border
        border.TopLine        = BorderLine(0,      11, 19, 19)
        border.BottomLine     = BorderLine(0xFF,   00, 11, 00)
        border.HorizontalLine = BorderLine(0xFF00, 00, 90, 00)
        xTable.setPropertyValue("TableBorder", border)
    # read set border
        border = xTable.getPropertyValue("TableBorder")

        self.assertTrue(border.IsTopLineValid)
        self.assertEqual(11, border.TopLine.InnerLineWidth)
        self.assertEqual(19, border.TopLine.OuterLineWidth)
        self.assertEqual(19, border.TopLine.LineDistance)
        self.assertEqual(0, border.TopLine.Color)

        self.assertTrue(border.IsBottomLineValid)
        self.assertEqual(0, border.BottomLine.InnerLineWidth)
        self.assertEqual(11, border.BottomLine.OuterLineWidth)
        self.assertEqual(0, border.BottomLine.LineDistance)
        self.assertEqual(0xFF, border.BottomLine.Color)

        self.assertTrue(border.IsLeftLineValid)
        self.assertEqual(0, border.LeftLine.InnerLineWidth)
        self.assertEqual(2, border.LeftLine.OuterLineWidth)
        self.assertEqual(0, border.LeftLine.LineDistance)
        self.assertEqual(0, border.LeftLine.Color)

        self.assertTrue(border.IsRightLineValid)
        self.assertEqual(0, border.RightLine.InnerLineWidth)
        self.assertEqual(2, border.RightLine.OuterLineWidth)
        self.assertEqual(0, border.RightLine.LineDistance)
        self.assertEqual(0, border.RightLine.Color)

        self.assertTrue(border.IsHorizontalLineValid)
        self.assertEqual(0, border.HorizontalLine.InnerLineWidth)
        self.assertEqual(90, border.HorizontalLine.OuterLineWidth)
        self.assertEqual(0, border.HorizontalLine.LineDistance)
        self.assertEqual(0xFF00, border.HorizontalLine.Color)

        self.assertTrue(border.IsVerticalLineValid)
        self.assertEqual(0, border.VerticalLine.InnerLineWidth)
        self.assertEqual(2, border.VerticalLine.OuterLineWidth)
        self.assertEqual(0, border.VerticalLine.LineDistance)
        self.assertEqual(0, border.VerticalLine.Color)

        self.assertTrue(border.IsDistanceValid)
        self.assertEqual(97, border.Distance)

        border2 = xTable.getPropertyValue("TableBorder2")
        self.assertTrue(border2.IsTopLineValid)
        self.assertEqual(11, border2.TopLine.InnerLineWidth)
        self.assertEqual(19, border2.TopLine.OuterLineWidth)
        self.assertEqual(19,  border2.TopLine.LineDistance)
        self.assertEqual(0, border2.TopLine.Color)
        self.assertEqual(DOUBLE, border2.TopLine.LineStyle)
        self.assertEqual(49, border2.TopLine.LineWidth)

        self.assertTrue(border2.IsBottomLineValid)
        self.assertEqual(0, border2.BottomLine.InnerLineWidth)
        self.assertEqual(11, border2.BottomLine.OuterLineWidth)
        self.assertEqual(0, border2.BottomLine.LineDistance)
        self.assertEqual(0xFF, border2.BottomLine.Color)
        self.assertEqual(SOLID, border2.BottomLine.LineStyle)
        self.assertEqual(11, border2.BottomLine.LineWidth)

        self.assertTrue(border2.IsLeftLineValid)
        self.assertEqual(0, border2.LeftLine.InnerLineWidth)
        self.assertEqual(2, border2.LeftLine.OuterLineWidth)
        self.assertEqual(0, border2.LeftLine.LineDistance)
        self.assertEqual(0, border2.LeftLine.Color)
        self.assertEqual(SOLID, border2.LeftLine.LineStyle)
        self.assertEqual(2, border2.LeftLine.LineWidth)

        self.assertTrue(border2.IsRightLineValid)
        self.assertEqual(0, border2.RightLine.InnerLineWidth)
        self.assertEqual(2, border2.RightLine.OuterLineWidth)
        self.assertEqual(0, border2.RightLine.LineDistance)
        self.assertEqual(0, border2.RightLine.Color)
        self.assertEqual(SOLID, border2.RightLine.LineStyle)
        self.assertEqual(2, border2.RightLine.LineWidth)

        self.assertTrue(border2.IsHorizontalLineValid)
        self.assertEqual(0, border2.HorizontalLine.InnerLineWidth)
        self.assertEqual(90, border2.HorizontalLine.OuterLineWidth)
        self.assertEqual(0, border2.HorizontalLine.LineDistance)
        self.assertEqual(0xFF00, border2.HorizontalLine.Color)
        self.assertEqual(SOLID, border2.HorizontalLine.LineStyle)
        self.assertEqual(90, border2.HorizontalLine.LineWidth)

        self.assertTrue(border2.IsVerticalLineValid)
        self.assertEqual(0, border2.VerticalLine.InnerLineWidth)
        self.assertEqual(2, border2.VerticalLine.OuterLineWidth)
        self.assertEqual(0, border2.VerticalLine.LineDistance)
        self.assertEqual(0, border2.VerticalLine.Color)
        self.assertEqual(SOLID, border2.VerticalLine.LineStyle)
        self.assertEqual(2, border2.VerticalLine.LineWidth)

        self.assertTrue(border2.IsDistanceValid)
        self.assertEqual(97, border2.Distance)
    # set border2
        border2.RightLine      = BorderLine2(0,      0, 0, 0, THICKTHIN_LARGEGAP, 120)
        border2.LeftLine       = BorderLine2(0,      0, 0, 0, EMBOSSED, 90)
        border2.VerticalLine   = BorderLine2(0xFF,   0, 90, 0, DOTTED, 0)
        border2.HorizontalLine = BorderLine2(0xFF00, 0, 0, 0, DASHED, 11)
        xTable.setPropertyValue("TableBorder2", border2)
    # read set border2
        border2 = xTable.getPropertyValue("TableBorder2")

        self.assertTrue(border2.IsTopLineValid)
        self.assertEqual(11, border2.TopLine.InnerLineWidth)
        self.assertEqual(19, border2.TopLine.OuterLineWidth)
        self.assertEqual(19, border2.TopLine.LineDistance)
        self.assertEqual(0, border2.TopLine.Color)
        self.assertEqual(DOUBLE, border2.TopLine.LineStyle)
        self.assertEqual(49, border2.TopLine.LineWidth)

        self.assertTrue(border2.IsBottomLineValid)
        self.assertEqual(0, border2.BottomLine.InnerLineWidth)
        self.assertEqual(11, border2.BottomLine.OuterLineWidth)
        self.assertEqual(0, border2.BottomLine.LineDistance)
        self.assertEqual(0xFF, border2.BottomLine.Color)
        self.assertEqual(SOLID, border2.BottomLine.LineStyle)
        self.assertEqual(11, border2.BottomLine.LineWidth)

        self.assertTrue(border2.IsLeftLineValid)
        self.assertEqual(23, border2.LeftLine.InnerLineWidth)
        self.assertEqual(23, border2.LeftLine.OuterLineWidth)
        self.assertEqual(46, border2.LeftLine.LineDistance)
        self.assertEqual(0, border2.LeftLine.Color)
        self.assertEqual(EMBOSSED, border2.LeftLine.LineStyle)
        self.assertEqual(90, border2.LeftLine.LineWidth)

        self.assertTrue(border2.IsRightLineValid)
        self.assertEqual(53, border2.RightLine.InnerLineWidth)
        self.assertEqual(26, border2.RightLine.OuterLineWidth)
        self.assertEqual(41, border2.RightLine.LineDistance)
        self.assertEqual(0, border2.RightLine.Color)
        self.assertEqual(THICKTHIN_LARGEGAP, border2.RightLine.LineStyle)
        self.assertEqual(120, border2.RightLine.LineWidth)

        self.assertTrue(border2.IsHorizontalLineValid)
        self.assertEqual(0, border2.HorizontalLine.InnerLineWidth)
        self.assertEqual(11, border2.HorizontalLine.OuterLineWidth)
        self.assertEqual(0, border2.HorizontalLine.LineDistance)
        self.assertEqual(0xFF00, border2.HorizontalLine.Color)
        self.assertEqual(DASHED, border2.HorizontalLine.LineStyle)
        self.assertEqual(11, border2.HorizontalLine.LineWidth)

        self.assertTrue(border2.IsVerticalLineValid)
        self.assertEqual(0, border2.VerticalLine.InnerLineWidth)
        self.assertEqual(90, border2.VerticalLine.OuterLineWidth)
        self.assertEqual(0, border2.VerticalLine.LineDistance)
        self.assertEqual(0xFF, border2.VerticalLine.Color)
        self.assertEqual(DOTTED, border2.VerticalLine.LineStyle)
        self.assertEqual(90, border2.VerticalLine.LineWidth)

        self.assertTrue(border2.IsDistanceValid)
        self.assertEqual(97, border2.Distance)

    # close document
        xDoc.dispose()

    def test_fdo58242(self):
        xDoc = CheckTable._uno.openEmptyWriterDoc()
    # insert table
        xTable = xDoc.createInstance("com.sun.star.text.TextTable")
        xTable.initialize(3, 3)
        xText = xDoc.getText()
        xCursor = xText.createTextCursor()
        xText.insertTextContent(xCursor, xTable, False)
    # get anchor
        xAnchor = xTable.getAnchor()

    # check all properties on the anchor - shouldn't crash despite
    # pointing to a non-SwTextNode
        xPropsInfo = xAnchor.getPropertySetInfo()
        for i in xPropsInfo.getProperties():
            try:
                xAnchor.getPropertyValue(i.Name)
            except RuntimeException:
                pass
    # close document
        xDoc.dispose()

    def test_descriptions(self):
        xDoc = CheckTable._uno.openEmptyWriterDoc()
        # insert table
        xTable = xDoc.createInstance("com.sun.star.text.TextTable")
        xTable.initialize(3, 3)
        xCursor = xDoc.Text.createTextCursor()
        xDoc.Text.insertTextContent(xCursor, xTable, False)
        self.assertEqual(3, xTable.Rows.Count)
        self.assertEqual(3, xTable.Columns.Count)
        # fill table
        self._fill_table(xTable)
        self._check_table(xTable)
        # check without labels first
        xTable.ChartColumnAsLabel = False
        xTable.ChartRowAsLabel = False
        self.assertEqual(0, len(xTable.RowDescriptions))
        self.assertEqual(0, len(xTable.ColumnDescriptions))
        self.RowDescriptions = ('foo', 'bar', 'baz') # no labels, thus noop
        self.ColumnDescriptions = ('foo', 'bar', 'baz') # no labels, thus noop
        self._check_table(xTable)
        # now check with labels
        xTable.ChartColumnAsLabel = True
        xTable.ChartRowAsLabel = True
        self.assertEqual(2, len(xTable.RowDescriptions))
        self.assertEqual('Cell 0 1', xTable.RowDescriptions[0])
        self.assertEqual('Cell 0 2', xTable.RowDescriptions[1])
        self.assertEqual(2, len(xTable.ColumnDescriptions))
        self.assertEqual('Cell 1 0', xTable.ColumnDescriptions[0])
        self.assertEqual('Cell 2 0', xTable.ColumnDescriptions[1])
        with self.assertRaises(Exception):
            xTable.RowDescriptions = ('foo',) # too short
        with self.assertRaises(Exception):
            xTable.ColumnDescriptions = ('foo',) # too short
        self._check_table(xTable)
        xTable.RowDescriptions = ('fooRow', 'bazRow')
        xTable.ColumnDescriptions = ('fooColumn', 'bazColumn')
        self.assertEqual('fooRow', xTable.getCellByPosition(0,1).String)
        self.assertEqual('bazRow', xTable.getCellByPosition(0,2).String)
        self.assertEqual('fooColumn', xTable.getCellByPosition(1,0).String)
        self.assertEqual('bazColumn', xTable.getCellByPosition(2,0).String)
        xTable.getCellByPosition(0,1).String = 'Cell 0 1' # reset changes values ...
        xTable.getCellByPosition(0,2).String = 'Cell 0 2'
        xTable.getCellByPosition(1,0).String = 'Cell 1 0'
        xTable.getCellByPosition(2,0).String = 'Cell 2 0'
        self._check_table(xTable) # ... to ensure the rest was untouched
        # check disconnected table excepts, but doesnt crash
        xTable2 = xDoc.createInstance("com.sun.star.text.TextTable")
        xTable2.initialize(3, 3)
        with self.assertRaises(Exception):
            foo = xTable2.RowDescriptions
        with self.assertRaises(Exception):
            foo = xTable2.ColumnDescriptions
        xDoc.dispose()

    def test_getset_data(self):
        xDoc = CheckTable._uno.openEmptyWriterDoc()
        # insert table
        xTable = xDoc.createInstance("com.sun.star.text.TextTable")
        xTable.initialize(4, 3)
        xCursor = xDoc.Text.createTextCursor()
        xDoc.Text.insertTextContent(xCursor, xTable, False)
        xTable.ChartColumnAsLabel = False
        xTable.ChartRowAsLabel = False
        # roundtrip
        xTable.Data = ((1,2,3), (4,5,6), (7,8,9), (10,11,12))
        self.assertEqual( xTable.Data, ((1,2,3), (4,5,6), (7,8,9), (10,11,12)))
        # missing row
        with self.assertRaises(Exception):
            xTable.Data = ((1,2,3), (4,5,6), (7,8,9))
        # missing column
        with self.assertRaises(Exception):
            xTable.Data = ((1,2), (4,5), (7,8), (10,11))
        # with labels
        xTable.ChartColumnAsLabel = True
        xTable.ChartRowAsLabel = True
        self.assertEqual( xTable.Data, ((5,6), (8,9), (11,12)))
        xTable.Data = ((55,66), (88,99), (1111,1212))
        xTable.ChartColumnAsLabel = True
        xTable.ChartRowAsLabel = False
        self.assertEqual( xTable.Data, ((2,3), (55,66), (88,99), (1111,1212)))
        xTable.ChartColumnAsLabel = False
        xTable.ChartRowAsLabel = True
        self.assertEqual( xTable.Data, ((4,55,66), (7,88,99), (10,1111,1212)))
        xTable.ChartColumnAsLabel = False
        xTable.ChartRowAsLabel = False
        self.assertEqual( xTable.Data, ((1,2,3), (4,55,66), (7,88,99), (10,1111,1212)))
        xDoc.dispose()

if __name__ == '__main__':
    unittest.main()

# /* vim:set shiftwidth=4 softtabstop=4 expandtab: */
