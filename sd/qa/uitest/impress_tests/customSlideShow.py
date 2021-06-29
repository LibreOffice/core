# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
import importlib
from uitest.debug import sleep
from uitest.uihelper.common import select_pos
from uitest.uihelper.common import get_state_as_dict, type_text

class customSlideShow(UITestCase):
    def test_customSlideShow(self):
        MainDoc = self.ui_test.create_doc_in_start_center("impress")
        MainWindow = self.xUITest.getTopFocusWindow()
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
#            print(DefineCustomSlideShow.getChildren())
                custompages = DefineCustomSlideShow.getChild("custompages")
                pages = DefineCustomSlideShow.getChild("pages")
                remove = DefineCustomSlideShow.getChild("remove")
#            print(get_state_as_dict(custompages))
                self.assertEqual(get_state_as_dict(custompages)["Children"], "1")
                self.assertEqual(get_state_as_dict(pages)["Children"], "1")
                xEntry = custompages.getChild("0")
                xEntry.executeAction("SELECT", tuple())
#            remove.executeAction("CLICK",tuple())  #tdf126951
                self.assertEqual(get_state_as_dict(custompages)["Children"], "1")
                self.assertEqual(get_state_as_dict(pages)["Children"], "1")

            delete = CustomSlideShows.getChild("delete")
            customshowlist = CustomSlideShows.getChild("customshowlist")
            self.assertEqual(get_state_as_dict(customshowlist)["Children"], "1")

#        delete.executeAction("CLICK",tuple())
#        self.assertEqual(get_state_as_dict(customshowlist)["Children"], "0")


        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
