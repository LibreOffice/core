#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#('CmisInfoPage', 'CmisScroll', 'CmisWindow', 'CustomInfoPage', 'DescriptionInfoPage', 'DocumentFontsPage', 'DocumentInfoPage', 'DocumentPropertiesDialog', 'SecurityInfoPage', 'StatisticsInfoPage', 'add', 'alignment1', 'box1', 'box2', 'cancel', 'changepass', 'comments', 'dialog-action_area1', 'dialog-vbox1', 'embedFonts', 'fontEmbeddingFrame', 'fontEmbeddingLabel', 'grid1', 'grid9', 'help', 'icon', 'keywords', 'label11', 'label13', 'label14', 'label15', 'label16', 'label17', 'label18', 'label27', 'label28', 'label29', 'label30', 'label31', 'label32', 'label33', 'label34', 'label35', 'label4', 'label47', 'label5', 'label6', 'label7', 'label8', 'lineft', 'name', 'nameed', 'nochars', 'nocharsexspaces', 'nogrfs', 'nolines', 'nooles', 'nopages', 'noparas', 'notables', 'nowords', 'ok', 'properties', 'protect', 'readonly', 'recordchanges', 'reset', 'showcreate', 'showedittime', 'showlocation', 'showmodify', 'showprint', 'showrevision', 'showsigned', 'showsize', 'showtemplate', 'showtype', 'signature', 'standard', 'subject', 'tabcontrol', 'templateft', 'thumbnailsavecb', 'title', 'type', 'unprotect', 'update', 'userdatacb', 'value', 'viewport1')
#Document properties dialog
#tdf 81457

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict
import time
from uitest.debug import sleep
from uitest.uihelper.common import select_pos


class documentProperties(UITestCase):

   def test_open_documentProperties_writer(self):
        self.ui_test.create_doc_in_start_center("writer")
        self.ui_test.execute_dialog_through_command(".uno:SetDocumentProperties")
        xDialog = self.xUITest.getTopFocusWindow()
        xResetBtn = xDialog.getChild("reset")
        xResetBtn.executeAction("CLICK", tuple())

        xUserDataCheckbox = xDialog.getChild("userdatacb")  # apply user data
        xUserDataCheckbox.executeAction("CLICK", tuple())
        xThumbSaveCheckbox = xDialog.getChild("thumbnailsavecb")  # save preview image with document
        xThumbSaveCheckbox.executeAction("CLICK", tuple())

#digital signature
        xDigSignBtn = xDialog.getChild("signature")                                                        
       
        def handle_sign_dlg(dialog):
            xNoBtn = dialog.getChild("no")
            self.ui_test.close_dialog_through_button(xNoBtn)                                        
       
        self.ui_test.execute_blocking_action(xDigSignBtn.executeAction, args=('CLICK', ()),              
                dialog_handler=handle_sign_dlg)            

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
        select_pos(xTabs, "5")     #tab Fonts
        xFontEmbedCheckbox = xDialog.getChild("embedFonts")
        xFontEmbedCheckbox.executeAction("CLICK", tuple())

#Security tab
        select_pos(xTabs, "4")     #tab Security
        xReadOnlyCheckbox = xDialog.getChild("readonly")
        xReadOnlyCheckbox.executeAction("CLICK", tuple())
        xRecordChangesCheckbox = xDialog.getChild("recordchanges")
        xRecordChangesCheckbox.executeAction("CLICK", tuple())
        xProtectBtn = xDialog.getChild("protect")   
        def handle_protect_dlg(dialog):
            xOkBtn = dialog.getChild("ok")
            xPasswordText = dialog.getChild("pass1ed")
            xPasswordText.executeAction("TYPE", mkPropertyValues({"TEXT":"password"}))
            xConfirmText = dialog.getChild("confirm1ed")
            xConfirmText.executeAction("TYPE", mkPropertyValues({"TEXT":"password"}))
            self.ui_test.close_dialog_through_button(xOkBtn)                                        
       
        self.ui_test.execute_blocking_action(xProtectBtn.executeAction, args=('CLICK', ()),              
                dialog_handler=handle_protect_dlg)            

        select_pos(xTabs, "2")     #tab Custom properties
#add custom properties  ------>>>>>>>>>>>  not supported
        xAddBtn = xDialog.getChild("add")
        xAddBtn.executeAction("CLICK", tuple())

        select_pos(xTabs, "6")     #tab Statistics
        xUpdateBtn = xDialog.getChild("update")
        xUpdateBtn.executeAction("CLICK", tuple())

        xOkBtn = xDialog.getChild("ok")
        xOkBtn.executeAction("CLICK", tuple())
#now open the dialog again and read the properties
        self.ui_test.execute_dialog_through_command(".uno:SetDocumentProperties")
        xDialog = self.xUITest.getTopFocusWindow()
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
        xCancBtn = xDialog.getChild("cancel")
        xCancBtn.executeAction("CLICK", tuple())




        self.ui_test.close_doc()


