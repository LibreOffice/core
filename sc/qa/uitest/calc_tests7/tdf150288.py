# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.uno.propertyvalue import mkPropertyValues
from libreoffice.calc.document import get_cell_by_position
from uitest.uihelper.common import select_by_text

class tdf150288(UITestCase):

    def test_tdf150288(self):

        with self.ui_test.create_doc_in_start_center("calc") as document:

            with self.ui_test.execute_dialog_through_command(".uno:OptionsTreeDialog") as xDialog:
                xPages = xDialog.getChild("pages")
                xLanguageEntry = xPages.getChild('2')
                xLanguageEntry.executeAction("EXPAND", tuple())
                xxLanguageEntryGeneralEntry = xLanguageEntry.getChild('0')
                xxLanguageEntryGeneralEntry.executeAction("SELECT", tuple())

                xDatePatterns = xDialog.getChild("datepatterns")
                xLocaleSetting = xDialog.getChild("localesetting")

                select_by_text(xLocaleSetting, "English (USA)")

                self.assertEqual("M/D/Y;M/D", get_state_as_dict(xDatePatterns)['Text'])

                xDatePatterns.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                xDatePatterns.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                xDatePatterns.executeAction("TYPE", mkPropertyValues({"TEXT":"D/M/Y;D/M"}))

            xCalcDoc = self.xUITest.getTopFocusWindow()
            xGridWin = xCalcDoc.getChild("grid_window")

            enter_text_to_cell(xGridWin, "A1", "31/8/22")
            enter_text_to_cell(xGridWin, "A2", "1/8/22")

            self.assertEqual("08/31/22", get_cell_by_position(document, 0, 0, 0).getString())

            # Without the fix in place, this test would have failed with
            # AssertionError: '08/01/22' != '01/08/22'
            self.assertEqual("08/01/22", get_cell_by_position(document, 0, 0, 1).getString())

# vim: set shiftwidth=4 softtabstop=4 expandtab:
