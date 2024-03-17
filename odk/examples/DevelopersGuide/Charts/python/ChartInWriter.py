# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import Helper
import ChartHelper

from com.sun.star.awt import Point, Size
from com.sun.star.drawing.FillStyle import SOLID


class ChartInWriter(object):
    def __init__(self, chart_doc):
        self._chart_document = chart_doc
        self._diagram = chart_doc.getDiagram()

    def lock_controllers(self):
        self._chart_document.lockControllers()

    def unlock_controllers(self):
        self._chart_document.unlockControllers()

    def test_wall(self):
        wall = self._diagram.getWall()
        # change background color of area
        wall.setPropertyValue("FillColor", int(0xEECC99))
        wall.setPropertyValue("FillStyle", SOLID)


# Test to create a writer document and insert an OLE Chart.
# Be careful!  This does not really work.  The Writer currently has no
# interface for dealing with OLE objects.  You can add an OLE shape to the
# Writer's drawing layer, but it is not treated correctly as OLE object.
# Thus, you can not activate the chart by double-clicking.  The office may
# also crash when the document is closed!
def main():
    helper = Helper.Helper()
    chart_helper = ChartHelper.ChartHelper(helper.create_text_document())

    # the unit for measures is 1/100th of a millimeter
    # position at (1cm, 1cm)
    pos = Point(1000, 1000)

    # size of the chart is 15cm x 13cm
    extent = Size(15000, 13000)

    # insert a new chart into the "Chart" sheet of the
    # spreadsheet document
    chart_doc = chart_helper.insert_ole_chart_in_writer(
        pos, extent, "com.sun.star.chart.AreaDiagram"
    )

    # instantiate test class with newly created chart
    test = ChartInWriter(chart_doc)

    try:
        test.lock_controllers()

        # do tests here
        test.test_wall()

        test.unlock_controllers()

    except Exception as err:
        print(f"UNO Exception caught: {err}")


# Main entry point
if __name__ == "__main__":
    main()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
