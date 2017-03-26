# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

def get_sheet_from_doc(document, index=None, name=None):
    """ Returns a sheet object for a Spreadsheet document

    Keyword arguments:
    index -- the 0-based index of the sheet (may not be used together with name)
    name -- the name of the sheet (may not be used together with index)
    """
    return document.getSheets().getByIndex(index)

def get_cell_by_position(document, tab, column, row):
    """ Get the cell object through its position in a document

    Keyword arguments:
    document -- The document that should be used
    tab -- The 0-based sheet number
    column -- The 0-based column number
    row -- THe 0-based row number
    """
    sheet = get_sheet_from_doc(document, tab)
    return sheet.getCellByPosition(column, row)

def get_column(document, column, tab = 0):
    """ Get the column object through the column index

    Keyword arguments:
    document -- The document that should be used
    tab -- The 0-based sheet number
    column -- The 0-based column number
    """
    sheet = get_sheet_from_doc(document, tab)
    return sheet.getColumns().getByIndex(column)

def get_row(document, row, tab = 0):
    """ Get the row object through the row index

    Keyword arguments:
    document -- The document that should be used
    tab -- The 0-based sheet number
    column -- The 0-based row number
    """
    sheet = get_sheet_from_doc(document, tab)
    return sheet.getRows().getByIndex(row)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
