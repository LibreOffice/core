# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import select_pos

class documentProperties(UITestCase):

   def assert_custom_properties(self, dialog, bIsFirstItemVisible):
        for i in range(6):
            aExpected = 'false'
            if bIsFirstItemVisible and i == 0:
                aExpected = 'true'
            xNameBox = dialog.getChild("namebox" + str(i + 1))
            xTypeBox = dialog.getChild("typebox" + str(i + 1))
            xValueEdit = dialog.getChild("valueedit" + str(i + 1))
            xRemoveBtn = dialog.getChild("remove" + str(i + 1))
            self.assertEqual(aExpected, get_state_as_dict(xNameBox)['ReallyVisible'])
            self.assertEqual(aExpected, get_state_as_dict(xTypeBox)['ReallyVisible'])
            self.assertEqual(aExpected, get_state_as_dict(xValueEdit)['ReallyVisible'])
            self.assertEqual(aExpected, get_state_as_dict(xRemoveBtn)['ReallyVisible'])

   def test_open_documentProperties_writer(self):
        with self.ui_test.create_doc_in_start_center("writer"):
            with self.ui_test.execute_dialog_through_command(".uno:SetDocumentProperties") as xDialog:
                xResetBtn = xDialog.getChild("reset")
                xResetBtn.executeAction("CLICK", tuple())

                xUserDataCheckbox = xDialog.getChild("userdatacb")  # apply user data
                xUserDataCheckbox.executeAction("CLICK", tuple())
                xThumbSaveCheckbox = xDialog.getChild("thumbnailsavecb")  # save preview image with document
                xThumbSaveCheckbox.executeAction("CLICK", tuple())

#digital signature
                xDigSignBtn = xDialog.getChild("signature")

                with self.ui_test.execute_blocking_action(xDigSignBtn.executeAction, args=('CLICK', ()), close_button="no"):
                    pass

                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "1")     #tab Description

                xTitleText = xDialog.getChild("title")
                xTitleText.executeAction("TYPE", mkPropertyValues({"TEXT":"Title text"}))
                xSubjectText = xDialog.getChild("subject")
                xSubjectText.executeAction("TYPE", mkPropertyValues({"TEXT":"Subject text"}))
                xKeywordsText = xDialog.getChild("keywords")
                xKeywordsText.executeAction("TYPE", mkPropertyValues({"TEXT":"Keywords text"}))
                xCommentsText = xDialog.getChild("comments")
                xCommentsText.executeAction("TYPE", mkPropertyValues({"TEXT":"Comments text"}))


#Font tab
                select_pos(xTabs, "4")     #tab Fonts
                xFontEmbedCheckbox = xDialog.getChild("embedFonts")
                xFontEmbedCheckbox.executeAction("CLICK", tuple())

#Security tab
                select_pos(xTabs, "3")     #tab Security
                xReadOnlyCheckbox = xDialog.getChild("readonly")
                xReadOnlyCheckbox.executeAction("CLICK", tuple())
                xRecordChangesCheckbox = xDialog.getChild("recordchanges")
                xRecordChangesCheckbox.executeAction("CLICK", tuple())
                xProtectBtn = xDialog.getChild("protect")

                with self.ui_test.execute_blocking_action(xProtectBtn.executeAction, args=('CLICK', ())) as dialog:
                    xPasswordText = dialog.getChild("pass1ed")
                    xPasswordText.executeAction("TYPE", mkPropertyValues({"TEXT":"password"}))
                    xConfirmText = dialog.getChild("confirm1ed")
                    xConfirmText.executeAction("TYPE", mkPropertyValues({"TEXT":"password"}))

                select_pos(xTabs, "2")     #tab Custom properties

                self.assert_custom_properties(xDialog, False)

                xAddBtn = xDialog.getChild("add")
                xAddBtn.executeAction("CLICK", tuple())

                self.assert_custom_properties(xDialog, True)

                xRemoveBtn = xDialog.getChild("remove1")
                xRemoveBtn.executeAction("CLICK", tuple())

                self.assert_custom_properties(xDialog, False)

                select_pos(xTabs, "5")     #tab Statistics
                xUpdateBtn = xDialog.getChild("update")
                xUpdateBtn.executeAction("CLICK", tuple())

#now open the dialog again and read the properties
            with self.ui_test.execute_dialog_through_command(".uno:SetDocumentProperties", close_button="cancel") as xDialog:
                xTitleText = xDialog.getChild("title")
                xSubjectText = xDialog.getChild("subject")
                xKeywordsText = xDialog.getChild("keywords")
                xCommentsText = xDialog.getChild("comments")
                xReadOnlyCheckbox = xDialog.getChild("readonly")
                xRecordChangesCheckbox = xDialog.getChild("recordchanges")
                xFontEmbedCheckbox = xDialog.getChild("embedFonts")
                xUserDataCheckbox = xDialog.getChild("userdatacb")
                xThumbSaveCheckbox = xDialog.getChild("thumbnailsavecb")
                self.assertEqual(get_state_as_dict(xTitleText)["Text"], "Title text")
                self.assertEqual(get_state_as_dict(xSubjectText)["Text"], "Subject text")
                self.assertEqual(get_state_as_dict(xKeywordsText)["Text"], "Keywords text")
                self.assertEqual(get_state_as_dict(xReadOnlyCheckbox)["Selected"], "true")
                self.assertEqual(get_state_as_dict(xRecordChangesCheckbox)["Selected"], "true")
                self.assertEqual(get_state_as_dict(xReadOnlyCheckbox)["Selected"], "true")
                self.assertEqual(get_state_as_dict(xFontEmbedCheckbox)["Selected"], "true")
                self.assertEqual(get_state_as_dict(xUserDataCheckbox)["Selected"], "false")
                self.assertEqual(get_state_as_dict(xThumbSaveCheckbox)["Selected"], "false")
                self.assertEqual(get_state_as_dict(xCommentsText)["Text"], "Comments text")
                xResetBtn = xDialog.getChild("reset")
                xResetBtn.executeAction("CLICK", tuple())


# vim: set shiftwidth=4 softtabstop=4 expandtab:
