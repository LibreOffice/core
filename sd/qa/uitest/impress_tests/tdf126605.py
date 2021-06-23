# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues

class tdf126605(UITestCase):

   def test_run(self):
        self.ui_test.create_doc_in_start_center("impress")
        xTemplateDlg = self.xUITest.getTopFocusWindow()
        xCancelBtn = xTemplateDlg.getChild("close")
        self.ui_test.close_dialog_through_button(xCancelBtn)

        xDoc = self.xUITest.getTopFocusWindow()
        xEdit = xDoc.getChild("impress_win")
        xEdit.executeAction("TYPE", mkPropertyValues({"TEXT":"test"}))

        # Without the accompanying fix in place, it would crash at this point
        self.xUITest.executeCommand(".uno:ParaRightToLeft")
        self.xUITest.executeCommand(".uno:ParaLeftToRight")

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
