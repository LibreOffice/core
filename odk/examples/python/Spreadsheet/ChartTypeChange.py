# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import sys
import time
import traceback
from typing import Union

import officehelper
from com.sun.star.awt import Rectangle
from com.sun.star.lang import IndexOutOfBoundsException


def main():
    try:
        remote_context = officehelper.bootstrap()
        srv_mgr = remote_context.getServiceManager()
        desktop = srv_mgr.createInstanceWithContext(
            "com.sun.star.frame.Desktop", remote_context
        )
        doc = desktop.loadComponentFromURL("private:factory/scalc", "_blank", 0, tuple())
        sheet = doc.Sheets[0]

        cell_values = (
            ("", "Jan", "Feb", "Mar", "Apr", "Mai"),
            ("Profit", 12.3, 43.2, 5.1, 76, 56.8),
            ("Rival in business", 12.2, 12.6, 17.7, 20.4, 100),
        )

        # Write the data into spreadsheet.
        for row, row_data in enumerate(cell_values):
            for column, cell_value in enumerate(row_data):
                insert_into_cell(column, row, cell_value, sheet)

        # Create a rectangle, which holds the size of the chart.
        rect = Rectangle()
        rect.X, rect.Y, rect.Width, rect.Height = 500, 3000, 25000, 11000

        # Create the Unicode of the character for the column name.
        char_rect = chr(65 + len(cell_values[0]) - 1)
        # Get the cell range of the written values.
        chart_cell_range = sheet[f"A1:{char_rect}{len(cell_values)}"]
        # Create a table chart with all written values
        chart_data_source = [chart_cell_range.RangeAddress]

        sheet.Charts.addNewByName("Example", rect, chart_data_source, True, True)

        # Get the newly created chart
        table_chart = sheet.Charts["Example"]

        # Change chart types one by one
        chart_types = (
            "com.sun.star.chart.LineDiagram",
            "com.sun.star.chart.BarDiagram",
            "com.sun.star.chart.PieDiagram",
            "com.sun.star.chart.NetDiagram",
            "com.sun.star.chart.XYDiagram",
            "com.sun.star.chart.StockDiagram",
            "com.sun.star.chart.AreaDiagram",
        )
        total = len(chart_types)

        for i, type_name in enumerate(chart_types, start=1):
            time.sleep(3)
            print("Change chart type to:", f"[{i}/{total}]", type_name)
            try:
                chart_doc = table_chart.EmbeddedObject
                chart_doc.Title.String = f"Chart Type: {type_name}"
                diagram = chart_doc.createInstance(type_name)
                diagram.Dim3D = False
                chart_doc.Diagram = diagram
            except Exception as e:
                print(f"Fail to change chart type to {type_name}: {e}", file=sys.stderr)
                traceback.print_exc()
    except Exception as e:
        print(f"Fail to change chart type: {e}", file=sys.stderr)
        traceback.print_exc()


def insert_into_cell(column: int, row: int, value: Union[str, float], sheet):
    try:
        cell = sheet[row, column]
    except IndexOutOfBoundsException:
        print("Could not get Cell", file=sys.stderr)
        traceback.print_exc()
    else:
        if isinstance(value, str):
            cell.String = value
        else:
            cell.Value = value


if __name__ == "__main__":
    main()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
