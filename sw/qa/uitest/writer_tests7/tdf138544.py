# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import select_pos
from uitest.uihelper.calc import enter_text_to_cell
from libreoffice.calc.document import get_cell_by_position
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict, type_text
from uitest.debug import sleep
import org.libreoffice.unotest
import pathlib
import time

def get_url_for_data_file(file_name):
    return pathlib.Path(org.libreoffice.unotest.makeCopyFromTDOC(file_name)).as_uri()

#LoadUserSettings - impact on Writer - Compatibility
#
# If you do not load the user settings from the document itself, then they should come from the program settings
# and not from some "old behaviour" settings
#
# Testing the effect of disabling Options - Load and Save - General - Load User settings from document
# on the user settings in Options - Writer - Compatibility
class tdf138544(UITestCase):

    def test_tdf138544(self):
        #writer_doc = self.ui_test.load_file(get_url_for_data_file("tdf138544.odt"))
        self.ui_test.create_doc_in_start_center("writer")

        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()
        self.ui_test.execute_dialog_through_command(".uno:OptionsTreeDialog")  #optionsdialog
        xDialog = self.xUITest.getTopFocusWindow()

        xPages = xDialog.getChild("pages")
        #xLoadAndSaveEntry = xPages.getChild('1')            # Load and Save
        #xLoadAndSaveEntry.executeAction("EXPAND", tuple())
        #xLoadAndSaveGeneralEntry = xLoadAndSaveEntry.getChild('0')  # General
        #xLoadAndSaveGeneralEntry.executeAction("SELECT", tuple())

        #xDialog = self.xUITest.getTopFocusWindow()
        #print(xDialog.getChildren())
        #print("==================================")
        #print(xLoadAndSaveEntry.getChildren())
        #loadUserSettings = xDialog.getChild("load_settings")
        #if (get_state_as_dict(loadUserSettings)["Selected"]) == "true":
        #    loadUserSettings.executeAction("CLICK", tuple())

        #time.sleep(10)


        xWriterEntry = xPages.getChild('3')                 # Writer
        xWriterEntry.executeAction("EXPAND", tuple())
        xWriterCompatibilityEntry = xWriterEntry.getChild('11')     # Compatibility
        xWriterCompatibilityEntry.executeAction("SELECT", tuple())
        xDialog = self.xUITest.getTopFocusWindow()

        print("============COMPAT======================")
        print(xDialog.getChildren())
        print("============optionsscroll=================")
        print(xWriterCompatibilityEntry.getChild("optionsscroll"))
        print("============format=================")
        print(xWriterCompatibilityEntry.getChild("format"))
        print("============format as dict=================")
        print(get_state_as_dict(xWriterCompatibilityEntry.getChild("format")))

        time.sleep(10)
        print(xWriterCompatibilityEntry.getChild("frame1").getChildren())
        xOKBtn = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKBtn)


        #self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
