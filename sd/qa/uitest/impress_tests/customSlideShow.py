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

class customSlideShow(UITestCase):
    def test_customSlideShow(self):
        with self.ui_test.create_doc_in_start_center("impress"):
            TemplateDialog = self.xUITest.getTopFocusWindow()
            cancel = TemplateDialog.getChild("close")
            self.ui_test.close_dialog_through_button(cancel)
            with self.ui_test.execute_dialog_through_command(".uno:CustomShowDialog") as CustomSlideShows:
                new = CustomSlideShows.getChild("new")

                with self.ui_test.execute_blocking_action(new.executeAction, args=('CLICK', ())) as DefineCustomSlideShow:
                    customname = DefineCustomSlideShow.getChild("customname")
                    customname.executeAction("TYPE", mkPropertyValues({"KEYCODE": "BACKSPACE"}))
                    customname.executeAction("TYPE", mkPropertyValues({"TEXT": "a"}))
                    customname.executeAction("TYPE", mkPropertyValues({"TEXT": "a"}))
                    pages = DefineCustomSlideShow.getChild("pages")
                    xEntry = pages.getChild("0")
                    xEntry.executeAction("SELECT", tuple())
                    add = DefineCustomSlideShow.getChild("add")
                    add.executeAction("CLICK",tuple())

            #verify
            with self.ui_test.execute_dialog_through_command(".uno:CustomShowDialog") as CustomSlideShows:
                edit = CustomSlideShows.getChild("edit")
                customshowlist = CustomSlideShows.getChild("customshowlist")
                self.assertEqual(get_state_as_dict(customshowlist)["SelectionCount"], "1")

                with self.ui_test.execute_blocking_action(edit.executeAction, args=('CLICK', ()), close_button="cancel") as DefineCustomSlideShow:
                    customname = DefineCustomSlideShow.getChild("customname")
                    self.assertEqual(get_state_as_dict(customname)["Text"], "aa")
                    custompages = DefineCustomSlideShow.getChild("custompages")
                    pages = DefineCustomSlideShow.getChild("pages")
                    self.assertEqual(get_state_as_dict(custompages)["Children"], "1")
                    self.assertEqual(get_state_as_dict(pages)["Children"], "1")
                    xEntry = custompages.getChild("0")
                    xEntry.executeAction("SELECT", tuple())
                    self.assertEqual(get_state_as_dict(custompages)["Children"], "1")
                    self.assertEqual(get_state_as_dict(pages)["Children"], "1")

                customshowlist = CustomSlideShows.getChild("customshowlist")
                self.assertEqual(get_state_as_dict(customshowlist)["Children"], "1")

    def test_tdf143125(self):
        with self.ui_test.create_doc_in_start_center("impress"):
            TemplateDialog = self.xUITest.getTopFocusWindow()
            cancel = TemplateDialog.getChild("close")
            self.ui_test.close_dialog_through_button(cancel)
            with self.ui_test.execute_dialog_through_command(".uno:CustomShowDialog"):
                # Without the fix in place, this test would have crashed here
                pass

# vim: set shiftwidth=4 softtabstop=4 expandtab:
