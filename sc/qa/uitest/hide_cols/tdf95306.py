# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase

from libreoffice.calc.document import get_column
from libreoffice.uno.propertyvalue import mkPropertyValues


def get_column_hidden(doc, index):
    column = get_column(doc, index)
    val = column.getPropertyValue("IsVisible")
    return not val

class HideDisjointColumns(UITestCase):

    def test_hide_columns(self):
        with self.ui_test.create_doc_in_start_center("calc") as doc:
            xTopWindow = self.xUITest.getTopFocusWindow()

            gridwin = xTopWindow.getChild("grid_window")
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "B1"}))
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "D1", "EXTEND":"true"}))

            self.assertFalse(get_column_hidden(doc, 1))
            self.assertFalse(get_column_hidden(doc, 3))
            self.ui_test._xUITest.executeCommand(".uno:HideColumn")
            self.assertTrue(get_column_hidden(doc, 1))
            self.assertTrue(get_column_hidden(doc, 3))
            self.ui_test._xUITest.executeCommand(".uno:Undo")
            self.assertFalse(get_column_hidden(doc, 1))
            self.assertFalse(get_column_hidden(doc, 3))


# vim: set shiftwidth=4 softtabstop=4 expandtab:
