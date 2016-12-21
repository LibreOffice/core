# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

def get_sheet_from_doc(document, index):
    return document.getSheets().getByIndex(index)

def get_cell_by_position(document, tab, column, row):
    sheet = get_sheet_from_doc(document, tab)
    return sheet.getCellByPosition(column, row)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
