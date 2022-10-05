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

class tdf135938(UITestCase):

    def test_tdf135938_cross_reference_update(self):
        with self.ui_test.create_doc_in_start_center("writer"):
            with self.ui_test.execute_modeless_dialog_through_command(".uno:InsertReferenceField", close_button="cancel") as xDialog:
                # Select set reference type
                xTreelistType = xDialog.getChild("type-ref")
                xTreeEntry = xTreelistType.getChild('0')
                self.assertEqual(get_state_as_dict(xTreeEntry)["Text"], "Set Reference")
                xTreeEntry.executeAction("SELECT", tuple())

                # Insert cross references
                xName = xDialog.getChild("name-ref")
                xName.executeAction("TYPE", mkPropertyValues({"TEXT": "ABC"}))
                xInsert = xDialog.getChild("ok")
                xInsert.executeAction("CLICK", tuple())
                xName.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
                xName.executeAction("TYPE", mkPropertyValues({"TEXT": "DEF"}))
                xInsert.executeAction("CLICK", tuple())

                # Select insert reference type
                xTreeEntry = xTreelistType.getChild('1')
                self.assertEqual(get_state_as_dict(xTreeEntry)["Text"], "Insert Reference")
                xTreeEntry.executeAction("SELECT", tuple())

                # Filter the cross references
                xFilter = xDialog.getChild("filter")
                xFilter.executeAction("TYPE", mkPropertyValues({"TEXT": "A"}))
                # Without the fix in place, this test would have failed with
                # AssertionError: 'ABC' != 'DEF', i.e., the text of the name field did not change
                self.assertEqual(get_state_as_dict(xName)["Text"], "ABC")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
