# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase

class tdf131581(UITestCase):

    def test_tdf131581(self):
        with self.ui_test.create_doc_in_start_center("writer"):

            self.xUITest.getTopFocusWindow()

            with self.ui_test.execute_dialog_through_command(".uno:OptionsTreeDialog") as xDialogOpt:

                xPages = xDialogOpt.getChild("pages")
                xLOEntry = xPages.getChild('0')
                xLOEntry.executeAction("EXPAND", tuple())
                xAdvancedEntry = xLOEntry.getChild('10')
                xAdvancedEntry.executeAction("SELECT", tuple())

                xExpertBtn = xDialogOpt.getChild("expertconfig")

                with self.ui_test.execute_blocking_action(xExpertBtn.executeAction, args=('CLICK', ())) as dialog:
                    # Without the fix in place, this would have hung
                    xSearchBtn = dialog.getChild("searchButton")
                    xSearchBtn.executeAction("CLICK", tuple())



# vim: set shiftwidth=4 softtabstop=4 expandtab:
