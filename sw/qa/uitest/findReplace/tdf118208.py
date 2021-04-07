# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file

#Find and replace
#tdf118208/118212 - enabling either CJK or CTL, or both (Tools -> Options -> Language Settings -> Languages: Default Languages for Documents checkboxes)
#eliminates all crashes. Setting back to Western only recreates the crashes.   - DONE

class tdf118208(UITestCase):

    def test_tdf118208_search_dialog_format_crash(self):
        writer_doc = self.ui_test.load_file(get_url_for_data_file("tdf118208.odt"))
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()
        # 1. Open the attached file.
        # 2. Press ctrl-H to show the search and replace dialog.
        # 3. Press the "Format..."  button.
        #Libreoffice immediately crashed.
        self.ui_test.execute_dialog_through_command(".uno:OptionsTreeDialog")  #optionsdialog
        xDialog = self.xUITest.getTopFocusWindow()

        xPages = xDialog.getChild("pages")
        xLanguageEntry = xPages.getChild('2')                 # Language Settings
        xLanguageEntry.executeAction("EXPAND", tuple())
        xxLanguageEntryGeneralEntry = xLanguageEntry.getChild('0')
        xxLanguageEntryGeneralEntry.executeAction("SELECT", tuple())          #Language

        asianlanguage = xDialog.getChild("asiansupport")
        complexlanguage = xDialog.getChild("ctlsupport")
        if (get_state_as_dict(asianlanguage)["Selected"]) == "true":
            asianlanguage.executeAction("CLICK", tuple())
        if (get_state_as_dict(complexlanguage)["Selected"]) == "true":
            complexlanguage.executeAction("CLICK", tuple())

        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        self.ui_test.execute_modeless_dialog_through_command(".uno:SearchDialog")
        xDialog = self.xUITest.getTopFocusWindow()

        format = xDialog.getChild("format")
        def handle_format_dlg(dialog):
            xOkBtn = dialog.getChild("ok")
            self.ui_test.close_dialog_through_button(xOkBtn)

        self.ui_test.execute_blocking_action(format.executeAction, args=('CLICK', ()),
                dialog_handler=handle_format_dlg)

        #verify
        self.assertEqual(document.Text.String[0:14], "Aaaaaaaaaaaaaa")
        xcloseBtn = xDialog.getChild("close")
        self.ui_test.close_dialog_through_button(xcloseBtn)

        #enable lang support again
        self.ui_test.execute_dialog_through_command(".uno:OptionsTreeDialog")  #optionsdialog
        xDialog = self.xUITest.getTopFocusWindow()

        xPages = xDialog.getChild("pages")
        xLanguageEntry = xPages.getChild('2')                 # Language Settings
        xLanguageEntry.executeAction("EXPAND", tuple())
        xxLanguageEntryGeneralEntry = xLanguageEntry.getChild('0')
        xxLanguageEntryGeneralEntry.executeAction("SELECT", tuple())          #Language

        asianlanguage = xDialog.getChild("asiansupport")
        complexlanguage = xDialog.getChild("ctlsupport")
        if (get_state_as_dict(asianlanguage)["Selected"]) == "false":
            asianlanguage.executeAction("CLICK", tuple())
        if (get_state_as_dict(complexlanguage)["Selected"]) == "false":
            complexlanguage.executeAction("CLICK", tuple())
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
