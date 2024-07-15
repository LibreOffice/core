# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.calc import enter_text_to_cell
from uitest.uihelper.common import get_state_as_dict

from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues


# Bug 125051 - EDITING: CRASH when start SpellCheck
class tdf125051(UITestCase):
    def test_tdf125051_crash_spelling_dialog(self):
        with self.ui_test.create_doc_in_start_center("calc") as document:
            xCalcDoc = self.xUITest.getTopFocusWindow()
            gridwin = xCalcDoc.getChild("grid_window")
            enter_text_to_cell(gridwin, "A1", "teext")
            gridwin.executeAction("SELECT", mkPropertyValues({"CELL": "A1"}))
            with self.ui_test.execute_dialog_through_command(".uno:SpellDialog", close_button="close") as xDialog:
                xSentence = xDialog.getChild("sentence")
                self.assertEqual("teext", get_state_as_dict(xSentence)['Text'])

                xLanguagelb = xDialog.getChild("languagelb")
                self.assertEqual("English (USA)", get_state_as_dict(xLanguagelb)["DisplayText"])

                xSuggestionslb = xDialog.getChild("suggestionslb")
                nSuggestions = len(xSuggestionslb.getChildren())
                self.assertGreaterEqual(nSuggestions, 1)
                self.assertEqual("text", get_state_as_dict(xSuggestionslb.getChild("0"))["Text"])
                for i in range(1, nSuggestions):
                    # Check the first suggestion is not duplicated
                    self.assertNotEqual("text", get_state_as_dict(xSuggestionslb.getChild(str(i)))["Text"])

                self.assertEqual("true", get_state_as_dict(xSuggestionslb.getChild("0"))["IsSelected"])

                for i in range(1, nSuggestions):
                    self.assertEqual("false", get_state_as_dict(xSuggestionslb.getChild(str(i)))["IsSemiTransparent"])

                xChangeBtn = xDialog.getChild("change")

                # Without the fix in place, this would have crashed here
                with self.ui_test.execute_blocking_action(xChangeBtn.executeAction, args=('CLICK', ())):
                    pass

                for i in range(nSuggestions):
                    self.assertEqual("false", get_state_as_dict(xSuggestionslb.getChild(str(i)))["IsSemiTransparent"])

            #verify it didn't crash and autocorrect worked
            self.assertEqual(get_cell_by_position(document, 0, 0, 0).getString(), "text")


# vim: set shiftwidth=4 softtabstop=4 expandtab:
