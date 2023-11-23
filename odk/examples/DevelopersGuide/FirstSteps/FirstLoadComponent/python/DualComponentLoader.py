# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import uno
import officehelper
import sys
import traceback
from com.sun.star.sheet.CellFlags import FORMULA

def main(ctx=None):
    try:
        if ctx is None:  # Execution triggered via Office client
            ctx = uno.getComponentContext()
            if ctx is None:
                print("ERROR: Python UNO runtime is absent.")
                sys.exit(1)
        srv_mgr = ctx.getServiceManager()
        desktop = srv_mgr.createInstanceWithContext("com.sun.star.frame.Desktop", ctx)
        spreadsheet_component = desktop.loadComponentFromURL("private:factory/scalc", "_blank", 0, tuple())
        spreadsheets = spreadsheet_component.getSheets()
        spreadsheets.insertNewByName("MySheet", 0)
        elem_type = spreadsheets.getElementType()
        print(elem_type)
        sheet = spreadsheets.getByName("MySheet")
        cell = sheet.getCellByPosition(0, 0)
        cell.setValue(21)
        cell = sheet.getCellByPosition(0, 1)
        cell.setValue(21)
        cell = sheet.getCellByPosition(0, 2)
        cell.setFormula("=sum(A1:A2)")

        cell.setPropertyValue("CellStyle", "Result")

        spreadsheet_controller = spreadsheet_component.getCurrentController()
        spreadsheet_controller.setActiveSheet(sheet)
        cell.setPropertyValue("VertJustify", "com.sun.star.table.CellVertJustify.TOP")
        formula_cells = sheet.queryContentCells(FORMULA)
        formulas = formula_cells.getCells()
        formula_enum = formulas.createEnumeration()

        while formula_enum.hasMoreElements():
            formula_cell = formula_enum.nextElement()
            print("Formula cell in column " + str(formula_cell.getCellAddress().Column)
                  + ", row " + str(formula_cell.getCellAddress().Row)
                  + " contains " + cell.getFormula())

    except Exception as e:
        print(e)
        traceback.print_exc()
        sys.exit(1)

# Entry point for LO client via
# Tools - Macros - Run macro... menu
g_exportedScripts = (main,)

# Entry point for Python console or Python IDEs
if __name__ == "__main__":
    remote_context = officehelper.bootstrap()
    if remote_context is None:
        print("ERROR: Could not bootstrap default Office.")
        sys.exit(1)
    main(remote_context)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
