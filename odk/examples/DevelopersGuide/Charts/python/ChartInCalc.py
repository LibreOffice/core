# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import os

import Helper
import CalcHelper

from com.sun.star.awt import FontWeight, Point, Size
from com.sun.star.chart import ChartSymbolType, ChartDataCaption
from com.sun.star.drawing import LineDash
from com.sun.star.drawing.DashStyle import ROUND
from com.sun.star.drawing.FillStyle import SOLID
from com.sun.star.drawing.LineStyle import DASH
from com.sun.star.lang import IndexOutOfBoundsException, Locale


class ChartInCalc(object):
    def __init__(self, chart_doc):
        super(ChartInCalc, self).__init__()
        self._chart_document = chart_doc
        self._diagram = self._chart_document.getDiagram()

    def lock_controllers(self):
        self._chart_document.lockControllers()

    def unlock_controllers(self):
        self._chart_document.unlockControllers()

    def test_diagram(self):
        dia_prop = self._diagram

        if dia_prop is not None:
            # change chart type
            dia_prop.setPropertyValue("Lines", True)
            # change attributes for all series
            # set line width to 0.5mm
            dia_prop.setPropertyValue("LineWidth", 50)

    def test_data_row_properties(self):
        # change properties of the data series
        try:
            for i in range(4):
                series_prop = self._diagram.getDataRowProperties(i)
                series_prop.setPropertyValue("LineColor", int(0x400000 * i + 0x005000 * i + 0x0000FF - 0x40 * i))

                if i == 1:
                    source_file = "bullet.gif"
                    url = os.path.abspath(source_file).replace("\\", "/")

                    if os.path.exists(url):
                        url = "file:///" + url
                    else:
                        url = "http://graphics.openoffice.org/chart/bullet1.gif"

                    series_prop.setPropertyValue("SymbolType", int(ChartSymbolType.BITMAPURL))
                    series_prop.setPropertyValue("SymbolBitmapURL", url)
                else:
                    series_prop.setPropertyValue("SymbolType", int(ChartSymbolType.SYMBOL1))
                    series_prop.setPropertyValue("SymbolSize", Size(250, 250))

        except IndexOutOfBoundsException as err:
            print(f"Oops, there not enough series for setting properties: {err}")

    def test_data_point_properties(self):
        #  set properties for a single data point
        try:
            # first parameter is the index of the point, the second one is the series
            point_prop = self._diagram.getDataPointProperties(0, 1)

            # set a different, larger symbol
            point_prop.setPropertyValue("SymbolType", int(ChartSymbolType.SYMBOL6))
            point_prop.setPropertyValue("SymbolSize", Size(600, 600))

            # add a label text with bold font, bordeaux red 14pt
            point_prop.setPropertyValue("DataCaption", int(ChartDataCaption.VALUE))
            point_prop.setPropertyValue("CharHeight", 14.0)
            point_prop.setPropertyValue("CharColor", int(0x993366))
            point_prop.setPropertyValue("CharWeight", FontWeight.BOLD)

        except IndexOutOfBoundsException as err:
            print(f"Oops, there not enough series for setting properties: {err}")

    def test_area(self):
        area = self._chart_document.getArea()
        if area is not None:
            # change background color of entire chart
            area.setPropertyValue("FillStyle", SOLID)
            area.setPropertyValue("FillColor", int(0xEEEEEE))

    def test_wall(self):
        wall = self._diagram.getWall()

        # change background color of area
        wall.setPropertyValue("FillStyle", SOLID)
        wall.setPropertyValue("FillColor", int(0xCCCCCC))

    def test_title(self):
        # change main title
        doc_prop = self._chart_document
        doc_prop.setPropertyValue("HasMainTitle", True)

        title = self._chart_document.getTitle()

        # set new text
        if title is not None:
            title.setPropertyValue("String", "Random Scatter Chart")
            title.setPropertyValue("CharHeight", 14.0)

        # align title with y axis
        axis = self._diagram.getYAxis()

        if axis is not None and title is not None:
            pos = title.getPosition()
            pos.X = axis.getPosition().X
            title.setPosition(pos)

    def test_axis(self):
        # x axis
        axis = self._diagram.getXAxis()

        if axis is not None:
            axis.setPropertyValue("Max", 24)
            axis.setPropertyValue("StepMain", 3)

        # change number format for y axis
        axis = self._diagram.getYAxis()

        # add a new custom number format and get the new key
        new_number_format = 0
        num_fmt_supp = self._chart_document

        if num_fmt_supp is not None:
            formats = num_fmt_supp.getNumberFormats()
            locale = Locale("de", "DE", "de")

            format_str = formats.generateFormat(new_number_format, locale, True, True, 3, 1)
            new_number_format = formats.addNew(format_str, locale)

        if axis is not None:
            axis.setPropertyValue("NumberFormat", int(new_number_format))

    def test_grid(self):
        # y major grid
        grid = self._diagram.getYMainGrid()

        if grid is not None:
            dash = LineDash()
            dash.Style = ROUND
            dash.Dots = 2
            dash.DotLen = 10
            dash.Dashes = 1
            dash.DashLen = 200
            dash.Distance = 100

            grid.setPropertyValue("LineColor", int(0x999999))
            grid.setPropertyValue("LineStyle", DASH)
            grid.setPropertyValue("LineDash", dash)
            grid.setPropertyValue("LineWidth", 30)


def main():
    # Create a spreadsheet add some data and add a chart
    helper = Helper.Helper()

    calc_helper = CalcHelper.CalcHelper(helper.create_spreadsheet_document())

    # insert a cell range with 4 columns and 24 rows filled with random numbers
    cell_range = calc_helper.insert_random_range(4, 24)
    range_address = cell_range.RangeAddress

    # change view to sheet containing the chart
    calc_helper.raise_chart_sheet()

    # the unit for measures is 1/100th of a millimeter
    # position at (1cm, 1cm)
    pos = Point(1000, 1000)

    # size of the chart is 15cm x 9.271cm
    extent = Size(15000, 9271)

    # insert a new chart into the "Chart" sheet of the spreadsheet document
    chart_doc = calc_helper.insert_chart("ScatterChart", range_address, pos, extent, "com.sun.star.chart.XYDiagram")

    test = ChartInCalc(chart_doc)

    try:
        test.lock_controllers()

        test.test_diagram()
        test.test_area()
        test.test_wall()
        test.test_title()
        test.test_axis()
        test.test_grid()

        # show an intermediate state, ...
        test.unlock_controllers()
        test.lock_controllers()

        # ..., because the following takes a while:
        # an internet URL has to be resolved
        test.test_data_row_properties()
        test.test_data_point_properties()

        test.unlock_controllers()

    except Exception as err:
        print(f"UNO Exception caught: {err}")


# Main entry point
if __name__ == "__main__":
    main()
