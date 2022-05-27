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
from libreoffice.uno.propertyvalue import mkPropertyValues

class tdf132097(UITestCase):

   def test_tdf132097(self):
        with self.ui_test.create_doc_in_start_center("calc"):
            with self.ui_test.execute_modeless_dialog_through_command(".uno:SearchDialog", close_button="close") as xDialog:
                xCols = xDialog.getChild('cols')
                xSearchTerm = xDialog.getChild("searchterm")
                xBackSearch = xDialog.getChild("backsearch")
                xSeachLabel = xDialog.getChild("searchlabel")

                xCols.executeAction("CLICK", tuple())
                xSearchTerm.executeAction("TYPE", mkPropertyValues({"TEXT":"TEST"}))

                for i in range(10):
                    # without the fix in place it would crash here.
                    # Sometimes it doesn't crash at first so try a few times to be sure
                    xBackSearch.executeAction("CLICK", tuple())

                self.assertEqual(get_state_as_dict(xSeachLabel)["Text"], "Search key not found")



# vim: set shiftwidth=4 softtabstop=4 expandtab:
