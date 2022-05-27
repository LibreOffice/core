# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file

from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues


# Bug 44861 - EDITING: result 'Find&Replace All' wrong for particular Regular Expression
class tdf44861(UITestCase):
   def test_tdf44861_find_replaceAll_regexp(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf44861.ods")) as calc_doc:
            # 2. ctrl-h, in dialog
            # Search: ([0-9]{2})([0-9]{2})
            # Replace: $1.$2
            # check option "Enable regular expressions"
            # Press "Replace all"
            with self.ui_test.execute_modeless_dialog_through_command(".uno:SearchDialog", close_button="close") as xDialog:
                searchterm = xDialog.getChild("searchterm")
                searchterm.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                searchterm.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
                searchterm.executeAction("TYPE", mkPropertyValues({"TEXT":"([0-9]{2})([0-9]{2})"}))
                replaceterm = xDialog.getChild("replaceterm")
                replaceterm.executeAction("TYPE", mkPropertyValues({"TEXT":"$1.$2"})) #replace textbox
                regexp = xDialog.getChild("regexp")
                regexp.executeAction("CLICK", tuple())
                self.assertEqual("true", get_state_as_dict(regexp)['Selected'])
                replaceall = xDialog.getChild("replaceall")
                replaceall.executeAction("CLICK", tuple())

                # Deselect regex button, otherwise it might affect other tests
                regexp.executeAction("CLICK", tuple())
                self.assertEqual("false", get_state_as_dict(regexp)['Selected'])

            #Expected: instead of 1345-1430 appears 13.45-14.30
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 0).getString(), "13.45-14.30")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 1).getString(), "13.45-14.30")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 2).getString(), "14.50-15.30")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 3).getString(), "12.55-13.35")
            self.assertEqual(get_cell_by_position(calc_doc, 0, 0, 4).getString(), "12.30-13.40")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
