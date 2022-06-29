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
from uitest.uihelper.common import select_pos

class Tdf148551(UITestCase):

    def test_tdf148551(self):

        with self.ui_test.create_doc_in_start_center("writer") as document:

            with self.ui_test.execute_modeless_dialog_through_command(".uno:InsertField", close_button="cancel") as xDialog:
                xTab = xDialog.getChild("tabcontrol")
                select_pos(xTab, "0")

                xDoc = xDialog.getChild("type-doc")
                xDoc.getChild('7').executeAction("SELECT", tuple())
                self.assertEqual("Statistics", get_state_as_dict(xDoc)['SelectEntryText'])

                xSelect = xDialog.getChild("select-doc")
                self.assertEqual("Pages", get_state_as_dict(xSelect)['SelectEntryText'])

                xFormat = xDialog.getChild("format-doc")
                self.assertEqual("As Page Style", get_state_as_dict(xFormat)['SelectEntryText'])

                xOk = xDialog.getChild("ok")
                xOk.executeAction("CLICK", tuple())

            # Without the fix in place, this test would have failed with
            # AssertionError: '1' != 'A'
            self.assertEqual("1", document.Text.createEnumeration().nextElement().String)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
