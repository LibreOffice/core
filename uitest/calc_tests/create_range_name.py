# -*- Mode: python; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from libreoffice.uno.propertyvalue import mkPropertyValues

from uitest.framework import UITestCase

class CreateRangeNameTest(UITestCase):

    def test_create_range_name(self):

        self.ui_test.create_doc_in_start_center("calc")

        self.ui_test.execute_modeless_dialog_through_command(".uno:AddName")

        xAddNameDlg = self.xUITest.getTopFocusWindow()

        props = {"TEXT": "simpleRangeName"}
        actionProps = mkPropertyValues(props)

        xEdit = xAddNameDlg.getChild("edit")
        xEdit.executeAction("TYPE", actionProps)
        xAddBtn = xAddNameDlg.getChild("add")
        xAddBtn.executeAction("CLICK", tuple())

        self.ui_test.close_doc()

    def test_create_local_range_name(self):

        self.ui_test.create_doc_in_start_center("calc")

        self.ui_test.execute_modeless_dialog_through_command(".uno:AddName")

        xAddNameDlg = self.xUITest.getTopFocusWindow()

        props = {"TEXT": "simpleRangeName"}
        actionProps = mkPropertyValues(props)

        xEdit = xAddNameDlg.getChild("edit")
        xEdit.executeAction("TYPE", actionProps)

        xScope = xAddNameDlg.getChild("scope")
        props = {"POS": "1"}
        scopeProps = mkPropertyValues(props)
        xScope.executeAction("SELECT", scopeProps)

        xAddBtn = xAddNameDlg.getChild("add")
        xAddBtn.executeAction("CLICK", tuple())

        self.ui_test.close_doc()

# vim:set shiftwidth=4 softtabstop=4 expandtab: */
