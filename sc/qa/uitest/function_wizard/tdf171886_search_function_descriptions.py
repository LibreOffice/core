# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, select_pos, type_text
from libreoffice.uno.propertyvalue import mkPropertyValues

class tdf171886(UITestCase):
    def test_tdf171886_search_function_descriptions(self):
        with self.ui_test.create_doc_in_start_center("calc"):
            with self.ui_test.execute_modeless_dialog_through_command(".uno:FunctionDialog") as xDialog:
                xFunctions = xDialog.getChild("function")

                # Select the "All" category and search for "k-th"
                select_pos(xDialog.getChild("category"), "2")
                type_text(xDialog.getChild("search"), "k-th")
                # Without the fix in place, this test would have failed with
                # AssertionError: '0' != '2'
                # i.e. the category "Text" with the function T was listed, since "k-th" contains T
                self.assertEqual("0", get_state_as_dict(xFunctions)["Children"])

                # Search also in the descriptions of the functions
                xSearchOptions = xDialog.getChild("searchoptions")
                xSearchOptions.executeAction("OPENFROMLIST", mkPropertyValues({"POS": "1"}))
                # Category "Statistical" with functions "LARGE" and "SMALL" is shown
                self.assertEqual("3", get_state_as_dict(xFunctions)["Children"])

                # Disable the search in the descriptions of the functions again
                xSearchOptions.executeAction("OPENFROMLIST", mkPropertyValues({"POS": "1"}))
                self.assertEqual("0", get_state_as_dict(xFunctions)["Children"])

# vim: set shiftwidth=4 softtabstop=4 expandtab:
