#! /usr/bin/env python
# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
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
from com.sun.star.beans import PropertyValue
from com.sun.star.uno import RuntimeException
from com.sun.star.table import BorderLine
from com.sun.star.table import BorderLine2
from com.sun.star.table.BorderLineStyle import (DOUBLE, SOLID, EMBOSSED,
                                                THICKTHIN_LARGEGAP, DASHED, DOTTED)
from com.sun.star.util import XNumberFormats
from com.sun.star.lang import Locale


class CheckTable(unittest.TestCase):

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
        cls.OOLineHairline = 2

    @classmethod
    def tearDownClass(cls):
        cls._uno.tearDown()

    def __test_borderAsserts(self, xBorderLine, line_valid):
        self.assertTrue(line_valid)
        self.assertEqual(0, xBorderLine.InnerLineWidth)
        self.assertEqual(self.OOLineHairline, xBorderLine.OuterLineWidth)
        self.assertEqual(0, xBorderLine.LineDistance)
        self.assertEqual(0, xBorderLine.Color)

    def __test_borderAssertsWithLineStyle(self, xBorderLine, line_valid):
        self.__test_borderAsserts(xBorderLine, line_valid)
        self.assertEqual(self.OOLineHairline, xBorderLine.LineWidth)
        self.assertEqual(SOLID, xBorderLine.LineStyle)

    def __test_borderDistance(self, border):
        self.assertTrue(border.IsDistanceValid)
        self.assertEqual(97, border.Distance)

    def test_tableborder(self):
        xDoc = CheckTable._uno.openEmptyWriterDoc()
        # insert table
        xTable = xDoc.createInstance("com.sun.star.text.TextTable")
        xTable.initialize(3, 3)
        xText = xDoc.getText()
        xCursor = xText.createTextCursor()
        xText.insertTextContent(xCursor, xTable, False)

        border_distances = xTable.TableBorderDistances

        self.assertEqual(97, border_distances.TopDistance)
        self.assertEqual(97, border_distances.BottomDistance)
        self.assertEqual(97, border_distances.LeftDistance)
        self.assertEqual(97, border_distances.RightDistance)

        self.assertEqual(True, border_distances.IsTopDistanceValid)
        self.assertEqual(True, border_distances.IsBottomDistanceValid)
        self.assertEqual(True, border_distances.IsLeftDistanceValid)
        self.assertEqual(True, border_distances.IsRightDistanceValid)

        border = xTable.getPropertyValue("TableBorder")

        self.__test_borderAsserts(border.TopLine, border.IsTopLineValid)
        self.__test_borderAsserts(border.BottomLine, border.IsBottomLineValid)
        self.__test_borderAsserts(border.LeftLine, border.IsLeftLineValid)
        self.__test_borderAsserts(border.RightLine, border.IsRightLineValid)
        self.__test_borderAsserts(border.HorizontalLine, border.IsHorizontalLineValid)
        self.__test_borderAsserts(border.VerticalLine, border.IsVerticalLineValid)

        self.__test_borderDistance(border)

    # set border
        border.TopLine = BorderLine(0,      11, 19, 19)
        border.BottomLine = BorderLine(0xFF,   00, 11, 00)
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

        self.__test_borderAsserts(border.LeftLine, border.IsLeftLineValid)

        self.__test_borderAsserts(border.RightLine, border.IsRightLineValid)

        self.assertTrue(border.IsHorizontalLineValid)
        self.assertEqual(0, border.HorizontalLine.InnerLineWidth)
        self.assertEqual(90, border.HorizontalLine.OuterLineWidth)
        self.assertEqual(0, border.HorizontalLine.LineDistance)
        self.assertEqual(0xFF00, border.HorizontalLine.Color)

        self.__test_borderAsserts(border.VerticalLine, border.IsVerticalLineValid)

        self.__test_borderDistance(border)

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

        self.__test_borderAssertsWithLineStyle(border2.LeftLine, border2.IsLeftLineValid)

        self.__test_borderAssertsWithLineStyle(border2.RightLine, border2.IsRightLineValid)

        self.assertTrue(border2.IsHorizontalLineValid)
        self.assertEqual(0, border2.HorizontalLine.InnerLineWidth)
        self.assertEqual(90, border2.HorizontalLine.OuterLineWidth)
        self.assertEqual(0, border2.HorizontalLine.LineDistance)
        self.assertEqual(0xFF00, border2.HorizontalLine.Color)
        self.assertEqual(SOLID, border2.HorizontalLine.LineStyle)
        self.assertEqual(90, border2.HorizontalLine.LineWidth)

        self.__test_borderAssertsWithLineStyle(border2.VerticalLine, border2.IsVerticalLineValid)

        self.__test_borderDistance(border2)
    # set border2
        border2.RightLine = BorderLine2(0, 0, 0, 0, THICKTHIN_LARGEGAP, 120)
        border2.LeftLine = BorderLine2(0, 0, 0, 0, EMBOSSED, 90)
        border2.VerticalLine = BorderLine2(0xFF, 0, 90, 0, DOTTED, 0)
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

        self.__test_borderDistance(border2)

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
        xTable.TableName = "foo"
        self.assertEqual("foo", xTable.TableName)
        xTable.TableTemplateName = "bar"
        self.assertEqual("bar", xTable.TableTemplateName)
        # fill table
        self._fill_table(xTable)
        self._check_table(xTable)
        # check without labels first
        xTable.ChartColumnAsLabel = False
        xTable.ChartRowAsLabel = False
        self.assertEqual(0, len(xTable.RowDescriptions))
        self.assertEqual(0, len(xTable.ColumnDescriptions))
        self.RowDescriptions = ('foo', 'bar', 'baz')  # no labels, thus noop
        self.ColumnDescriptions = ('foo', 'bar', 'baz')  # no labels, thus noop
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
            xTable.RowDescriptions = ('foo',)  # too short
        with self.assertRaises(Exception):
            xTable.ColumnDescriptions = ('foo',)  # too short
        self._check_table(xTable)
        xTable.RowDescriptions = ('fooRow', 'bazRow')
        xTable.ColumnDescriptions = ('fooColumn', 'bazColumn')
        self.assertEqual('fooRow', xTable.getCellByPosition(0, 1).String)
        self.assertEqual('bazRow', xTable.getCellByPosition(0, 2).String)
        self.assertEqual('fooColumn', xTable.getCellByPosition(1, 0).String)
        self.assertEqual('bazColumn', xTable.getCellByPosition(2, 0).String)
        xTable.getCellByPosition(0, 1).String = 'Cell 0 1'  # reset changes values ...
        xTable.getCellByPosition(0, 2).String = 'Cell 0 2'
        xTable.getCellByPosition(1, 0).String = 'Cell 1 0'
        xTable.getCellByPosition(2, 0).String = 'Cell 2 0'
        self._check_table(xTable)  # ... to ensure the rest was untouched
        # check disconnected table excepts, but doesn't crash
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
        xTable.Data = ((1, 2, 3), (4, 5, 6), (7, 8, 9), (10, 11, 12))
        self.assertEqual(xTable.Data, ((1, 2, 3), (4, 5, 6), (7, 8, 9), (10, 11, 12)))
        # missing row
        with self.assertRaises(Exception):
            xTable.Data = ((1, 2, 3), (4, 5, 6), (7, 8, 9))
        # missing column
        with self.assertRaises(Exception):
            xTable.Data = ((1, 2), (4, 5), (7, 8), (10, 11))
        # with labels
        xTable.ChartColumnAsLabel = True
        xTable.ChartRowAsLabel = True
        self.assertEqual(xTable.Data, ((5, 6), (8, 9), (11, 12)))
        xTable.Data = ((55, 66), (88, 99), (1111, 1212))
        xTable.ChartColumnAsLabel = True
        xTable.ChartRowAsLabel = False
        self.assertEqual(xTable.Data, ((2, 3), (55, 66), (88, 99), (1111, 1212)))
        xTable.ChartColumnAsLabel = False
        xTable.ChartRowAsLabel = True
        self.assertEqual(xTable.Data, ((4, 55, 66), (7, 88, 99), (10, 1111, 1212)))
        xTable.ChartColumnAsLabel = False
        xTable.ChartRowAsLabel = False
        self.assertEqual(xTable.Data, ((1, 2, 3), (4, 55, 66), (7, 88, 99), (10, 1111, 1212)))
        xDoc.dispose()

    def test_remove_colrow(self):
        xDoc = CheckTable._uno.openEmptyWriterDoc()
        xTable = xDoc.createInstance("com.sun.star.text.TextTable")
        xTable.initialize(4, 3)
        xCursor = xDoc.Text.createTextCursor()
        xDoc.Text.insertTextContent(xCursor, xTable, False)
        xTable.ChartColumnAsLabel = False
        xTable.ChartRowAsLabel = False
        xTable.Data = ((1, 2, 3), (4, 5, 6), (7, 8, 9), (10, 11, 12))
        xRows = xTable.Rows
        self.assertEqual(xRows.ImplementationName, 'SwXTableRows')
        self.assertTrue(xRows.supportsService('com.sun.star.text.TableRows'))
        self.assertFalse(xRows.supportsService('foo'))
        self.assertIn('com.sun.star.text.TableRows', xRows.SupportedServiceNames)
        self.assertNotIn('foo', xRows.SupportedServiceNames)
        xRows.removeByIndex(1, 2)
        self.assertEqual(xTable.Data, ((1, 2, 3), (10, 11, 12)))
        xCols = xTable.Columns
        self.assertEqual(xCols.ImplementationName, 'SwXTableColumns')
        self.assertTrue(xCols.supportsService('com.sun.star.text.TableColumns'))
        self.assertFalse(xCols.supportsService('foo'))
        self.assertIn('com.sun.star.text.TableColumns', xCols.SupportedServiceNames)
        self.assertNotIn('foo', xCols.SupportedServiceNames)
        xCols.removeByIndex(1, 1)
        self.assertEqual(xTable.Data, ((1, 3), (10, 12)))
        xDoc.dispose()

    def test_insert_colrow(self):
        xDoc = CheckTable._uno.openEmptyWriterDoc()
        xTable = xDoc.createInstance("com.sun.star.text.TextTable")
        xTable.initialize(4, 3)
        xCursor = xDoc.Text.createTextCursor()
        xDoc.Text.insertTextContent(xCursor, xTable, False)
        xTable.ChartColumnAsLabel = False
        xTable.ChartRowAsLabel = False
        xTable.Data = ((1, 2, 3), (4, 5, 6), (7, 8, 9), (10, 11, 12))
        xRows = xTable.Rows
        xRows.insertByIndex(1, 2)
        nan = float('nan')
        self.assertEqual(xTable.Data[0], (1, 2, 3))
        self.assertEqual(xTable.Data[3], (4, 5, 6))
        self.assertEqual(xTable.Data[4], (7, 8, 9))
        self.assertEqual(xTable.Data[5], (10, 11, 12))
        for x in range(3):
            self.assertTrue(math.isnan(xTable.Data[1][x]))
            self.assertTrue(math.isnan(xTable.Data[2][x]))
        xCols = xTable.Columns
        xCols.insertByIndex(1, 1)
        self.assertEqual(xTable.Data[0][0], 1)
        self.assertTrue(math.isnan(xTable.Data[0][1]))
        self.assertEqual(xTable.Data[0][2], 2)
        self.assertEqual(xTable.Data[0][3], 3)
        self.assertEqual(xTable.Data[3][0], 4)
        self.assertTrue(math.isnan(xTable.Data[3][1]))
        self.assertEqual(xTable.Data[3][2], 5)
        self.assertEqual(xTable.Data[3][3], 6)
        self.assertEqual(xTable.Data[4][0], 7)
        self.assertTrue(math.isnan(xTable.Data[4][1]))
        self.assertEqual(xTable.Data[4][2], 8)
        self.assertEqual(xTable.Data[4][3], 9)
        self.assertEqual(xTable.Data[5][0], 10)
        self.assertTrue(math.isnan(xTable.Data[5][1]))
        self.assertEqual(xTable.Data[5][2], 11)
        self.assertEqual(xTable.Data[5][3], 12)
        for x in range(4):
            self.assertTrue(math.isnan(xTable.Data[1][x]))
            self.assertTrue(math.isnan(xTable.Data[2][x]))
        xDoc.dispose()

    def test_chartdataprovider(self):
        xDoc = CheckTable._uno.openEmptyWriterDoc()
        xTable = xDoc.createInstance("com.sun.star.text.TextTable")
        xTable.initialize(4, 3)
        xCursor = xDoc.Text.createTextCursor()
        xDoc.Text.insertTextContent(xCursor, xTable, False)
        xTable.ChartColumnAsLabel = False
        xTable.ChartRowAsLabel = False
        xTable.Data = ((1, 2, 3), (4, 5, 6), (7, 8, 9), (10, 11, 12))
        self.assertTrue(xTable.Name == 'Table1')
        self.assertIn('com.sun.star.text.GenericTextDocument', xDoc.SupportedServiceNames)
        xChartdataprovider = xDoc.createInstance('com.sun.star.chart2.data.DataProvider')
        self.assertEqual(xChartdataprovider.ImplementationName, 'SwChartDataProvider')
        self.assertTrue(xChartdataprovider.supportsService('com.sun.star.chart2.data.DataProvider'))
        self.assertFalse(xChartdataprovider.supportsService('foo'))
        self.assertIn('com.sun.star.chart2.data.DataProvider', xChartdataprovider.SupportedServiceNames)
        pv = PropertyValue()
        pv.Name = 'CellRangeRepresentation'
        pv.Value = 'Table1.A1:C2'
        xDataSource = xChartdataprovider.createDataSource((pv,))
        self.assertEqual(len(xDataSource.DataSequences), 3)
        expected_values = ((1, 4), (2, 5), (3, 6))
        expected_cellrange = ('A1:A2', 'B1:B2', 'C1:C2')

        for col in range(3):
            xSeq = xDataSource.DataSequences[col].Values
            self.assertEqual(xSeq.ImplementationName, 'SwChartDataSequence')
            self.assertTrue(xSeq.supportsService('com.sun.star.chart2.data.DataSequence'))
            self.assertFalse(xSeq.supportsService('foo'))
            self.assertIn('com.sun.star.chart2.data.DataSequence', xSeq.SupportedServiceNames)
            self.assertEqual(xSeq.SourceRangeRepresentation, 'Table1.%s' % expected_cellrange[col])
            self.assertEqual(xSeq.Data, expected_values[col])
            self.assertEqual(xSeq.NumericalData, expected_values[col])
            self.assertEqual(
                    [int(txtval) for txtval in xSeq.TextualData],
                    [val for val in expected_values[col]])

            xSeq.Role = "One xSeq to rule them all"
            self.assertEqual("One xSeq to rule them all", xSeq.Role)

            xSeqClone = xSeq.createClone()
            self.assertEqual(xSeq.Role, xSeqClone.Role)

        xDoc.dispose()

    def test_tdf32082(self):
        xDoc = CheckTable._uno.openEmptyWriterDoc()
        xDocFrame = xDoc.CurrentController.Frame
        xContext = CheckTable._uno.getContext()
        xServiceManager = xContext.ServiceManager
        xDispatcher = xServiceManager.createInstanceWithContext(
            'com.sun.star.frame.DispatchHelper', xContext)
        xTable = xDoc.createInstance("com.sun.star.text.TextTable")
        xTable.initialize(1, 1)
        xCursor = xDoc.Text.createTextCursor()
        xDoc.Text.insertTextContent(xCursor, xTable, False)
        # Setup numberformat for the cell
        xNumberFormats = xDoc.NumberFormats
        xLocale = Locale('en', 'US', '')
        format_string = '#,##0.00 [$€-407];[RED]-#,##0.00 [$€-407]'
        key = xNumberFormats.queryKey(format_string, xLocale, True)
        if key == -1:
            key = xNumberFormats.addNew(format_string, xLocale)
        # Apply the format on the first cell
        xTable.getCellByPosition(0, 0).NumberFormat = key
        xDispatcher.executeDispatch(xDocFrame, '.uno:GoToStartOfDoc', '', 0, ())
        xDispatcher.executeDispatch(xDocFrame, '.uno:InsertText', '', 0,
                                    (PropertyValue('Text', 0, '3', 0),))
        xDispatcher.executeDispatch(xDocFrame, '.uno:JumpToNextCell', '', 0, ())
        # Check that the formatting we set up is not destroyed
        self.assertEqual(xTable.getCellByPosition(0, 0).getString(), '3.00 €')
        self.assertEqual(xTable.getCellByPosition(0, 0).getValue(), 3)
        # Verify that it works with number recognition turned on as well
        xDispatcher.executeDispatch(xDocFrame, '.uno:TableNumberRecognition', '', 0,
                                    (PropertyValue('TableNumberRecognition', 0, True, 0),))
        xDispatcher.executeDispatch(xDocFrame, '.uno:InsertText', '', 0,
                                    (PropertyValue('Text', 0, '4', 0),))
        xDispatcher.executeDispatch(xDocFrame, '.uno:JumpToNextCell', '', 0, ())
        self.assertEqual(xTable.getCellByPosition(0, 1).getString(), '4.00 €')
        self.assertEqual(xTable.getCellByPosition(0, 1).getValue(), 4)
        xDoc.dispose()

    def test_numberRecognition(self):
        xDoc = CheckTable._uno.openEmptyWriterDoc()
        xDocFrame = xDoc.CurrentController.Frame
        xContext = CheckTable._uno.getContext()
        xServiceManager = xContext.ServiceManager
        xDispatcher = xServiceManager.createInstanceWithContext(
            'com.sun.star.frame.DispatchHelper', xContext)
        xTable = xDoc.createInstance('com.sun.star.text.TextTable')
        xTable.initialize(2, 1)
        xCursor = xDoc.Text.createTextCursor()
        xDoc.Text.insertTextContent(xCursor, xTable, False)
        xDispatcher.executeDispatch(xDocFrame, '.uno:GoToStartOfDoc', '', 0, ())
        xDispatcher.executeDispatch(xDocFrame, '.uno:InsertText', '', 0,
                                    (PropertyValue('Text', 0, '2015-10-30', 0),))
        xDispatcher.executeDispatch(xDocFrame, '.uno:JumpToNextCell', '', 0, ())
        # Without number recognition 2015-10-30 should not be interpreted as a date
        self.assertEqual(xTable.getCellByPosition(0, 0).getString(), '2015-10-30')
        self.assertEqual(xTable.getCellByPosition(0, 0).getValue(), 0)
        # Activate number recognition
        xDispatcher.executeDispatch(xDocFrame, '.uno:TableNumberRecognition', '', 0,
                                    (PropertyValue('TableNumberRecognition', 0, True, 0),))
        xDispatcher.executeDispatch(xDocFrame, '.uno:InsertText', '', 0,
                                    (PropertyValue('Text', 0, '2015-10-30', 0),))
        xDispatcher.executeDispatch(xDocFrame, '.uno:JumpToNextCell', '', 0, ())
        # With number recognition it should now be a date, confirm by checking
        # the string and value of the cell.
        self.assertEqual(xTable.getCellByPosition(0, 1).getString(), '2015-10-30')
        self.assertEqual(xTable.getCellByPosition(0, 1).getValue(), 42307.0)
        xDoc.dispose()

    def test_tableTemplate(self):
        xDoc = CheckTable._uno.openEmptyWriterDoc()
        xTable = xDoc.createInstance("com.sun.star.text.TextTable")
        xTable.initialize(1, 1)
        xTable.setPropertyValue("TableTemplateName", "Default")
        self.assertEqual(xTable.getPropertyValue("TableTemplateName"), "Default")
        xTable.setPropertyValue("TableTemplateName", "other_style")
        self.assertEqual(xTable.getPropertyValue("TableTemplateName"), "other_style")
        xTable.setPropertyValue("TableTemplateName", "")
        self.assertEqual(xTable.getPropertyValue("TableTemplateName"), "")
        xDoc.dispose()


    def test_unoNames(self):
        xDoc = CheckTable._uno.openEmptyWriterDoc()
        xTable = xDoc.createInstance("com.sun.star.text.TextTable")
        xTable.initialize(3, 3)
        xText = xDoc.getText()
        xCursor = xText.createTextCursor()
        xText.insertTextContent(xCursor, xTable, False)

        self.assertEqual("SwXTextTable", xTable.ImplementationName)
        self.assertEqual(("com.sun.star.document.LinkTarget",
                          "com.sun.star.text.TextTable",
                          "com.sun.star.text.TextContent",
                          "com.sun.star.text.TextSortable"), xTable.SupportedServiceNames)
        self.assertEqual(b'', xTable.ImplementationId.value)

        xCell = xTable.getCellByPosition(1, 1)
        self.assertEqual("SwXCell", xCell.ImplementationName)
        self.assertEqual(("com.sun.star.text.CellProperties",), xCell.SupportedServiceNames)
        self.assertEqual(b'', xCell.ImplementationId.value)

        xRow = xTable.Rows[0]
        self.assertEqual("SwXTextTableRow", xRow.ImplementationName)
        self.assertEqual(("com.sun.star.text.TextTableRow",), xRow.SupportedServiceNames)
        self.assertEqual(b'', xRow.ImplementationId.value)

        xTableCursor = xTable.createCursorByCellName("A1")
        self.assertEqual("SwXTextTableCursor", xTableCursor.ImplementationName)
        self.assertEqual(("com.sun.star.text.TextTableCursor",), xTableCursor.SupportedServiceNames)
        self.assertEqual(b'', xTableCursor.ImplementationId.value)

        xDoc.dispose()

    def test_xmlRangeConversions(self):
        xDoc = CheckTable._uno.openEmptyWriterDoc()
        xTable = xDoc.createInstance("com.sun.star.text.TextTable")
        xTable.initialize(4, 3)
        xCursor = xDoc.Text.createTextCursor()
        xDoc.Text.insertTextContent(xCursor, xTable, False)
        xTable.ChartColumnAsLabel = False
        xTable.ChartRowAsLabel = False
        xTable.Data = ((1, 2, 3), (4, 5, 6), (7, 8, 9), (10, 11, 12))

        xChartDataProvider = xDoc.createInstance('com.sun.star.chart2.data.DataProvider')

        self.assertEqual('', xChartDataProvider.convertRangeToXML(''))
        self.assertEqual('', xChartDataProvider.convertRangeFromXML(''))
        self.assertEqual('.A1;.A1', xChartDataProvider.convertRangeFromXML('<some xml>'))

        xml = xChartDataProvider.convertRangeToXML('Table1.A1:C3')

        self.assertEqual("Table1.$A$1:.$C$3", xml)

        xCellRangeString = xChartDataProvider.convertRangeFromXML("Table1.$A$1:.$C$3")
        self.assertEqual("Table1.A1:C3", xCellRangeString)

    def test_splitRangeHorizontal(self):
        xDoc = CheckTable._uno.openEmptyWriterDoc()
        xTable = xDoc.createInstance("com.sun.star.text.TextTable")
        xTable.initialize(2, 2)
        xText = xDoc.getText()
        xCursor = xText.createTextCursor()
        xText.insertTextContent(xCursor, xTable, False)
        xTable.Data = ((1, 2), (3, 4))
        xCursor = xTable.createCursorByCellName("A1")
        xCursor.splitRange(2, True)
        self.assertEqual(len(xTable.Data), 4)
        self.assertEqual(xTable.Data[0], (float(1), float(2)))
        self.assertEqual(xTable.Data[3], (float(3), float(4)))
        self.assertTrue(math.isnan(xTable.Data[1][0]))
        self.assertTrue(math.isnan(xTable.Data[1][1]))
        self.assertTrue(math.isnan(xTable.Data[2][0]))
        self.assertTrue(math.isnan(xTable.Data[2][1]))

    def test_mergeRangeHorizontal(self):
        xDoc = CheckTable._uno.openEmptyWriterDoc()
        xTable = xDoc.createInstance("com.sun.star.text.TextTable")
        xTable.initialize(3, 3)
        xText = xDoc.getText()
        xCursor = xText.createTextCursor()
        xText.insertTextContent(xCursor, xTable, False)
        xTable.Data = ((1, 2, 3), (4, 5, 6), (7, 8, 9))
        xCursor = xTable.createCursorByCellName("A1")
        xCursor.goDown(1, True)
        xCursor.mergeRange()
        self.assertEqual(len(xTable.Data), 3)
        self.assertEqual(xTable.Data[0], (float(1), float(2), float(3)))
        self.assertTrue(math.isnan(xTable.Data[1][0]))
        self.assertEqual(xTable.Data[1][1], float(5))
        self.assertEqual(xTable.Data[1][2], float(6))
        self.assertEqual(xTable.Data[2], (float(7), float(8), float(9)))

if __name__ == '__main__':
    unittest.main()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
