'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Sub Main
    desktop = createUnoService("com.sun.star.frame.Desktop")
    Dim args()
    spreadsheet_component = desktop.loadComponentFromURL("private:factory/scalc", "_blank", 0, args())

    spreadsheets = spreadsheet_component.getSheets()
    spreadsheets.insertNewByName("MySheet", 0)
    elem_type = spreadsheets.getElementType()
    Msgbox(elem_type.Name)
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
    formula_cells = sheet.queryContentCells(com.sun.star.sheet.CellFlags.FORMULA)
    formulas = formula_cells.getCells()
    formula_enum = formulas.createEnumeration()

    Do while formula_enum.hasMoreElements()
        formula_cell = formula_enum.nextElement()
        Msgbox("Formula cell in column " + formula_cell.getCellAddress().Column + _
                     ", row " + formula_cell.getCellAddress().Row + _
                      " contains " + cell.getFormula())
    Loop
End Sub
