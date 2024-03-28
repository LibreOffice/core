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

import officehelper
from com.sun.star.awt import Rectangle
from com.sun.star.lang import IndexOutOfBoundsException

"""
Step 1: get the remote component context from the office
Step 2: open an empty calc document
Step 3: create cell styles
Step 4: get the sheet an insert some data
Step 5: apply the created cell styles
Step 6: insert a 3D Chart
"""


def main():
    # oooooooooooooooooooooooooooStep 1oooooooooooooooooooooooooooooooooooooooooo
    # call UNO bootstrap method and get the remote component context form
    # the a running office (office will be started if necessary)
    try:
        remote_context = officehelper.bootstrap()
        print("Connected to a running office ...")
        srv_mgr = remote_context.getServiceManager()
        desktop = srv_mgr.createInstanceWithContext(
            "com.sun.star.frame.Desktop", remote_context
        )
    # removing bare except: and handling it similar to the try/except already
    # committed in other parts of this script
    except Exception as e:
        print(f"Couldn't get Sheet: {e}")
        traceback.print_exc()
        sys.exit(1)

    # oooooooooooooooooooooooooooStep 2oooooooooooooooooooooooooooooooooooooooooo
    # open an empty document. In this case it's a calc document.
    # For this purpose an instance of com.sun.star.frame.Desktop
    # is created. The desktop provides the XComponentLoader interface,
    # which is used to open the document via loadComponentFromURL
    print("Opening an empty Calc document")
    doc_url = "private:factory/scalc"
    try:
        doc = desktop.loadComponentFromURL(doc_url, "_blank", 0, tuple())
    # removing bare except: and handling it similar to the try/except already
    # committed in other parts of this script
    except Exception as e:
        print(f"Couldn't get Sheet: {e}")
        traceback.print_exc()
        return

    # oooooooooooooooooooooooooooStep 3oooooooooooooooooooooooooooooooooooooooooo
    # create cell styles.
    # For this purpose get the StyleFamiliesSupplier and the family CellStyle.
    # Create an instance of com.sun.star.style.CellStyle and add it to the family.
    # Now change some properties

    try:
        cell_styles = doc.StyleFamilies["CellStyles"]
        cell_style = doc.createInstance("com.sun.star.style.CellStyle")
        cell_styles["My Style"] = cell_style
        cell_style.IsCellBackgroundTransparent = False
        cell_style.CellBackColor = 6710932
        cell_style.CharColor = 16777215
        cell_style = doc.createInstance("com.sun.star.style.CellStyle")
        cell_styles["My Style2"] = cell_style
        cell_style.IsCellBackgroundTransparent = False
        cell_style.CellBackColor = 13421823
    # removing bare except: and handling it similar to the try/except already
    # committed in other parts of this script
    except Exception as e:
        print(f"Couldn't get Sheet: {e}")
        traceback.print_exc()

    # oooooooooooooooooooooooooooStep 4oooooooooooooooooooooooooooooooooooooooooo
    # get the sheet an insert some data.
    # Get the sheets from the document and then the first from this container.
    # Now some data can be inserted. For this purpose get a Cell via
    # getCellByPosition and insert into this cell via setValue() (for floats)
    # or setFormula() for formulas and Strings.
    # As a Python example, those calls are made in equivalent Pythonic ways.

    print("Getting spreadsheet")
    try:
        sheet = doc.Sheets[0]
    except Exception as e:
        print(f"Couldn't get Sheet: {e}")
        traceback.print_exc()
        sys.exit(1)

    print("Creating the Header")

    insert_into_cell(1, 0, "JAN", sheet, "")
    insert_into_cell(2, 0, "FEB", sheet, "")
    insert_into_cell(3, 0, "MAR", sheet, "")
    insert_into_cell(4, 0, "APR", sheet, "")
    insert_into_cell(5, 0, "MAI", sheet, "")
    insert_into_cell(6, 0, "JUN", sheet, "")
    insert_into_cell(7, 0, "JUL", sheet, "")
    insert_into_cell(8, 0, "AUG", sheet, "")
    insert_into_cell(9, 0, "SEP", sheet, "")
    insert_into_cell(10, 0, "OCT", sheet, "")
    insert_into_cell(11, 0, "NOV", sheet, "")
    insert_into_cell(12, 0, "DEC", sheet, "")
    insert_into_cell(13, 0, "SUM", sheet, "")

    print("Fill the lines")

    insert_into_cell(0, 1, "Smith", sheet, "")
    insert_into_cell(1, 1, "42", sheet, "V")
    insert_into_cell(2, 1, "58.9", sheet, "V")
    insert_into_cell(3, 1, "-66.5", sheet, "V")
    insert_into_cell(4, 1, "43.4", sheet, "V")
    insert_into_cell(5, 1, "44.5", sheet, "V")
    insert_into_cell(6, 1, "45.3", sheet, "V")
    insert_into_cell(7, 1, "-67.3", sheet, "V")
    insert_into_cell(8, 1, "30.5", sheet, "V")
    insert_into_cell(9, 1, "23.2", sheet, "V")
    insert_into_cell(10, 1, "-97.3", sheet, "V")
    insert_into_cell(11, 1, "22.4", sheet, "V")
    insert_into_cell(12, 1, "23.5", sheet, "V")
    insert_into_cell(13, 1, "=SUM(B2:M2)", sheet, "")

    insert_into_cell(0, 2, "Jones", sheet, "")
    insert_into_cell(1, 2, "21", sheet, "V")
    insert_into_cell(2, 2, "40.9", sheet, "V")
    insert_into_cell(3, 2, "-57.5", sheet, "V")
    insert_into_cell(4, 2, "-23.4", sheet, "V")
    insert_into_cell(5, 2, "34.5", sheet, "V")
    insert_into_cell(6, 2, "59.3", sheet, "V")
    insert_into_cell(7, 2, "27.3", sheet, "V")
    insert_into_cell(8, 2, "-38.5", sheet, "V")
    insert_into_cell(9, 2, "43.2", sheet, "V")
    insert_into_cell(10, 2, "57.3", sheet, "V")
    insert_into_cell(11, 2, "25.4", sheet, "V")
    insert_into_cell(12, 2, "28.5", sheet, "V")
    insert_into_cell(13, 2, "=SUM(B3:M3)", sheet, "")

    insert_into_cell(0, 3, "Brown", sheet, "")
    insert_into_cell(1, 3, "31.45", sheet, "V")
    insert_into_cell(2, 3, "-20.9", sheet, "V")
    insert_into_cell(3, 3, "-117.5", sheet, "V")
    insert_into_cell(4, 3, "23.4", sheet, "V")
    insert_into_cell(5, 3, "-114.5", sheet, "V")
    insert_into_cell(6, 3, "115.3", sheet, "V")
    insert_into_cell(7, 3, "-171.3", sheet, "V")
    insert_into_cell(8, 3, "89.5", sheet, "V")
    insert_into_cell(9, 3, "41.2", sheet, "V")
    insert_into_cell(10, 3, "71.3", sheet, "V")
    insert_into_cell(11, 3, "25.4", sheet, "V")
    insert_into_cell(12, 3, "38.5", sheet, "V")
    insert_into_cell(13, 3, "=SUM(A4:L4)", sheet, "")

    # oooooooooooooooooooooooooooStep 5oooooooooooooooooooooooooooooooooooooooooo
    # apply the created cell style.
    # For this purpose get the PropertySet of the Cell and change the
    # property CellStyle to the appropriate value.

    change_backcolor(1, 0, 13, 0, "My Style", sheet)
    change_backcolor(0, 1, 0, 3, "My Style", sheet)
    change_backcolor(1, 1, 13, 3, "My Style2", sheet)

    # oooooooooooooooooooooooooooStep 6oooooooooooooooooooooooooooooooooooooooooo
    # insert a 3D chart.
    # get the CellRange which holds the data for the chart and its RangeAddress
    # get the TableChartSupplier from the sheet and then the TableCharts from it.
    # add a new chart based on the data to the TableCharts.
    # get the ChartDocument, which provide the Diagram. Change the properties
    # Dim3D (3 dimension) and String (the title) of the diagram.

    rect = Rectangle()
    rect.X, rect.Y, rect.Width, rect.Height = 500, 3000, 25000, 11000

    print("Insert Chart")
    data_range = (sheet["A1:N4"].RangeAddress,)
    sheet.Charts.addNewByName("Example", rect, data_range, True, True)

    # get the diagram and change some of the properties
    try:
        chart = sheet.Charts["Example"]
        # chart object implements XEmbeddedObjectSupplier interface
        diagram = chart.EmbeddedObject.Diagram
        print("Change Diagram to 3D")
        diagram.Dim3D = True

        print("Change the title")
        time.sleep(.2)
        chart.EmbeddedObject.Title.String = "The new title"
    except Exception as e:
        print(f"Changing Properties failed: {e}", file=sys.stderr)
        traceback.print_exc()

    print("done")


def insert_into_cell(column: int, row: int, value: str, sheet, flag: str):
    try:
        cell = sheet[row, column]
    except IndexOutOfBoundsException:
        print("Could not get Cell", file=sys.stderr)
        traceback.print_exc()
    else:
        if flag == "V":
            cell.Value = float(value)
        else:
            cell.Formula = value


def change_backcolor(left: int, top: int, right: int, bottom: int, template: str, sheet):
    try:
        cell_range = sheet[top:bottom + 1, left:right + 1]
        cell_range.CellStyle = template
    except IndexOutOfBoundsException:
        print("Could not get CellRange", file=sys.stderr)
        traceback.print_exc()
    except Exception as e:
        print(f"Can't change colors chgbColor: {e}", file=sys.stderr)
        traceback.print_exc()


if __name__ == "__main__":
    main()


# vim: set shiftwidth=4 softtabstop=4 expandtab:
