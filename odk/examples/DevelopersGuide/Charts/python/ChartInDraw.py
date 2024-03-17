# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import math

import Helper
import ChartHelper

from com.sun.star.awt import Point, Size
from com.sun.star.drawing.FillStyle import SOLID
from com.sun.star.drawing import Direction3D, HomogenMatrix, HomogenMatrixLine
from com.sun.star.chart.ChartLegendPosition import LEFT
from com.sun.star.chart.ChartSolidType import CYLINDER


class ChartInDraw(object):

    def __init__(self, chart_doc):
        self._chart_document = chart_doc
        self._diagram = chart_doc.getDiagram()

    def lock_controllers(self):
        self._chart_document.lockControllers()

    def unlock_controllers(self):
        self._chart_document.unlockControllers()

    def test_area(self):
        area = self._chart_document.getArea()
        if area is not None:
            # change background color of entire chart
            area.setPropertyValue("FillStyle", SOLID)
            area.setPropertyValue("FillColor", int(0xEEEEEE))

    def test_wall(self):
        wall = self._diagram.getWall()
        # change background color of area
        wall.setPropertyValue("FillColor", int(0xCCCCCC))
        wall.setPropertyValue("FillStyle", SOLID)

    def test_title(self):
        self._chart_document.setPropertyValue("HasMainTitle", True)

        title = self._chart_document.getTitle()
        if title is not None:
            title.setPropertyValue("String", "Bar Chart in a Draw Document")

    def test_legend(self):
        legend = self._chart_document.getLegend()
        legend.setPropertyValue("Alignment", LEFT)
        legend.setPropertyValue("FillStyle", SOLID)
        legend.setPropertyValue("FillColor", int(0xEEDDEE))

    def test_threeD(self):
        self._diagram.setPropertyValue("Dim3D", True)
        self._diagram.setPropertyValue("Deep", True)
        # from Chart3DBarProperties:
        self._diagram.setPropertyValue("SolidType", int(CYLINDER))

        # change floor color to Magenta6
        diagram_floor = self._diagram.getFloor()
        diagram_floor.setPropertyValue("FillColor", int(0x6B2394))

        # apply changes to get a 3d scene
        self._chart_document.unlockControllers()
        self._chart_document.lockControllers()

        # rotate scene to a different angle
        matrix = HomogenMatrix()
        lines = [
            HomogenMatrixLine(1.0, 0.0, 0.0, 0.0),
            HomogenMatrixLine(0.0, 1.0, 0.0, 0.0),
            HomogenMatrixLine(0.0, 0.0, 1.0, 0.0),
            HomogenMatrixLine(0.0, 0.0, 0.0, 1.0),
        ]

        matrix.Line1 = lines[0]
        matrix.Line2 = lines[1]
        matrix.Line3 = lines[2]
        matrix.Line4 = lines[3]

        # rotate 10 degrees along the x axis
        angle = 10.0
        cos_x = math.cos(math.pi / 180.0 * angle)
        sin_x = math.sin(math.pi / 180.0 * angle)

        # rotate -20 degrees along the y axis
        angle = -20.0
        cos_y = math.cos(math.pi / 180.0 * angle)
        sin_y = math.sin(math.pi / 180.0 * angle)

        # rotate -5 degrees along the z axis
        angle = -5.0
        cos_z = math.cos(math.pi / 180.0 * angle)
        sin_z = math.sin(math.pi / 180.0 * angle)

        line1 = HomogenMatrixLine(cos_y * cos_z, cos_y * -sin_z, sin_y, 0.0)
        line2 = HomogenMatrixLine(
            sin_x * sin_y * cos_z + cos_x * sin_z,
            -sin_x * sin_y * sin_z + cos_x * cos_z,
            -sin_x * cos_y,
            0.0,
        )
        line3 = HomogenMatrixLine(
            -cos_x * sin_y * cos_z + sin_x * sin_z,
            cos_x * sin_y * sin_z + sin_x * cos_z,
            cos_x * cos_y,
            0.0,
        )
        matrix.Line1 = line1
        matrix.Line2 = line2
        matrix.Line3 = line3
        self._diagram.setPropertyValue("D3DTransformMatrix", matrix)

        # add a red light source

        # in a chart by default only the second (non-specular) light source is switched on
        # light source 1 is a specular light source
        self._diagram.setPropertyValue("D3DSceneLightColor1", int(0xFF3333))

        # set direction
        direction = Direction3D()
        direction.DirectionX = -0.75
        direction.DirectionY = 0.5
        direction.DirectionZ = 0.5

        self._diagram.setPropertyValue("D3DSceneLightDirection1", direction)
        self._diagram.setPropertyValue("D3DSceneLightOn1", True)


# Create a spreadsheet add some data and add a chart
def main():
    helper = Helper.Helper()
    chart_helper = ChartHelper.ChartHelper(helper.create_drawing_document())

    # the unit for measures is 1/100th of a millimeter
    # position at (1cm, 1cm)
    pos = Point(1000, 1000)

    # size of the chart is 15cm x 13cm
    extent = Size(15000, 13000)

    # insert a new chart into the "Chart" sheet of the
    # spreadsheet document
    chart_doc = chart_helper.insert_ole_chart_in_draw(
        pos, extent, "com.sun.star.chart.BarDiagram"
    )

    # instantiate test class with newly created chart
    test = ChartInDraw(chart_doc)

    try:
        test.lock_controllers()

        test.test_area()
        test.test_wall()
        test.test_title()
        test.test_legend()
        test.test_threeD()

        test.unlock_controllers()

    except Exception as err:
        print(f"UNO Exception caught: {err}")


# Main entry point
if __name__ == "__main__":
    main()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
