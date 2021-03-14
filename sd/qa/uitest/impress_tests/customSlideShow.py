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
        self.ui_test.execute_dialog_through_command(".uno:CustomShowDialog")
        CustomSlideShows = self.xUITest.getTopFocusWindow()
        new = CustomSlideShows.getChild("new")
        ok = CustomSlideShows.getChild("ok")

        def handle_new_dlg(DefineCustomSlideShow):
            customname = DefineCustomSlideShow.getChild("customname")
            customname.executeAction("TYPE", mkPropertyValues({"KEYCODE": "BACKSPACE"}))
            customname.executeAction("TYPE", mkPropertyValues({"TEXT": "a"}))
            customname.executeAction("TYPE", mkPropertyValues({"TEXT": "a"}))
            pages = DefineCustomSlideShow.getChild("pages")
            xEntry = pages.getChild("0")
            xEntry.executeAction("SELECT", tuple())
            add = DefineCustomSlideShow.getChild("add")
            add.executeAction("CLICK",tuple())
            ok = DefineCustomSlideShow.getChild("ok")
            self.ui_test.close_dialog_through_button(ok)

        self.ui_test.execute_blocking_action(new.executeAction, args=('CLICK', ()),
                dialog_handler=handle_new_dlg)
        self.ui_test.close_dialog_through_button(ok)
        #verify
        self.ui_test.execute_dialog_through_command(".uno:CustomShowDialog")
        CustomSlideShows = self.xUITest.getTopFocusWindow()
        edit = CustomSlideShows.getChild("edit")
        customshowlist = CustomSlideShows.getChild("customshowlist")
        self.assertEqual(get_state_as_dict(customshowlist)["SelectionCount"], "1")
        ok = CustomSlideShows.getChild("ok")

        def handle_edit_dlg(DefineCustomSlideShow):
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
            cancel = DefineCustomSlideShow.getChild("cancel")
            self.ui_test.close_dialog_through_button(cancel)

        self.ui_test.execute_blocking_action(edit.executeAction, args=('CLICK', ()),
                dialog_handler=handle_edit_dlg)

        CustomSlideShows = self.xUITest.getTopFocusWindow()
        delete = CustomSlideShows.getChild("delete")
        customshowlist = CustomSlideShows.getChild("customshowlist")
        self.assertEqual(get_state_as_dict(customshowlist)["Children"], "1")

#        delete.executeAction("CLICK",tuple())
#        self.assertEqual(get_state_as_dict(customshowlist)["Children"], "0")

        ok = CustomSlideShows.getChild("ok")
        self.ui_test.close_dialog_through_button(ok)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
