# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict
import time
from uitest.debug import sleep
from uitest.uihelper.common import select_pos

class autocorrectOptions(UITestCase):

   def test_autocorrect_options_writer(self):
        self.ui_test.create_doc_in_start_center("writer")
        document = self.ui_test.get_component()
        xWriterDoc = self.xUITest.getTopFocusWindow()
        xWriterEdit = xWriterDoc.getChild("writer_edit")

        self.ui_test.execute_dialog_through_command(".uno:AutoCorrectDlg")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "0")       #tab replace
        origtext = xDialog.getChild("origtext")
        newtext = xDialog.getChild("newtext")
        xnew = xDialog.getChild("new")
        xdelete = xDialog.getChild("delete")
        xtabview = xDialog.getChild("tabview")
        xreset = xDialog.getChild("reset")
        nrRows = get_state_as_dict(xtabview)["VisibleCount"]

        #add new rule
        origtext.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        origtext.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        origtext.executeAction("TYPE", mkPropertyValues({"TEXT":"::::"}))
        newtext.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        newtext.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        newtext.executeAction("TYPE", mkPropertyValues({"TEXT":"dvojtecky"}))
        xnew.executeAction("CLICK", tuple())
        nrRowsNew = get_state_as_dict(xtabview)["VisibleCount"]
        nrRowsDiff = int(nrRowsNew) - int(nrRows)
        self.assertEqual(nrRowsDiff, 1)  #we have +1 rule
        #delete rule
        origtext.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        origtext.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        origtext.executeAction("TYPE", mkPropertyValues({"TEXT":"::::"}))
        newtext.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        newtext.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        newtext.executeAction("TYPE", mkPropertyValues({"TEXT":"dvojtecky"}))
        xdelete.executeAction("CLICK", tuple())
        self.assertEqual(get_state_as_dict(xtabview)["VisibleCount"], nrRows)   #we have default nr of rules

        select_pos(xTabs, "1")     #tab Exceptions
        #abbreviations
        abbrev = xDialog.getChild("abbrev")
        newabbrev = xDialog.getChild("newabbrev")
        delabbrev = xDialog.getChild("delabbrev")
        abbrevlist = xDialog.getChild("abbrevlist")

        nrRowsAbb = get_state_as_dict(abbrevlist)["Children"]
        abbrev.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        abbrev.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        abbrev.executeAction("TYPE", mkPropertyValues({"TEXT":"qqqqq"}))
        newabbrev.executeAction("CLICK", tuple())
        nrRowsAbbNew = get_state_as_dict(abbrevlist)["Children"]
        nrRowsAbbDiff = int(nrRowsAbbNew) - int(nrRowsAbb)
        self.assertEqual(nrRowsAbbDiff, 1)  #we have +1 rule
        delabbrev.executeAction("CLICK", tuple())
        self.assertEqual(get_state_as_dict(abbrevlist)["Children"], nrRowsAbb)   #we have default nr of rules

        #words with two initial capitals
        double = xDialog.getChild("double")
        newdouble = xDialog.getChild("newdouble")
        deldouble = xDialog.getChild("deldouble")
        doublelist = xDialog.getChild("doublelist")

        nrRowsDouble = get_state_as_dict(doublelist)["Children"]
        double.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        double.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        double.executeAction("TYPE", mkPropertyValues({"TEXT":"QQqqq"}))
        newdouble.executeAction("CLICK", tuple())
        nrRowsDoubleNew = get_state_as_dict(doublelist)["Children"]
        nrRowsDoubleDiff = int(nrRowsDoubleNew) - int(nrRowsDouble) #convert string and
        self.assertEqual(nrRowsDoubleDiff, 1)  #we have +1 rule
        deldouble.executeAction("CLICK", tuple())
        self.assertEqual(get_state_as_dict(doublelist)["Children"], nrRowsDouble)   #we have default nr of rules

        #tab Word Completion + tdf#95068
        select_pos(xTabs, "4")
        enablewordcomplete = xDialog.getChild("enablewordcomplete")
        appendspace = xDialog.getChild("appendspace")
        showastip = xDialog.getChild("showastip")
        whenclosing = xDialog.getChild("whenclosing")
        acceptwith = xDialog.getChild("acceptwith")
        minwordlen = xDialog.getChild("minwordlen")
        maxentries = xDialog.getChild("maxentries")

        if (get_state_as_dict(enablewordcomplete)["Selected"]) == "false":
            enablewordcomplete.executeAction("CLICK", tuple())
        appendspace.executeAction("CLICK", tuple())
        showastip.executeAction("CLICK", tuple())
        whenclosing.executeAction("CLICK", tuple())
        props = {"TEXT": "Right"}
        actionProps = mkPropertyValues(props)
        acceptwith.executeAction("SELECT", actionProps)
        minwordlen.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        minwordlen.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        minwordlen.executeAction("TYPE", mkPropertyValues({"TEXT":"6"}))
        maxentries.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        maxentries.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        maxentries.executeAction("TYPE", mkPropertyValues({"TEXT":"100"}))

        xOKButton = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKButton)
        #verify
        self.ui_test.execute_dialog_through_command(".uno:AutoCorrectDlg")
        xDialog = self.xUITest.getTopFocusWindow()
        xTabs = xDialog.getChild("tabcontrol")
        select_pos(xTabs, "4")
        enablewordcomplete = xDialog.getChild("enablewordcomplete")
        appendspace = xDialog.getChild("appendspace")
        showastip = xDialog.getChild("showastip")
        whenclosing = xDialog.getChild("whenclosing")
        acceptwith = xDialog.getChild("acceptwith")
        minwordlen = xDialog.getChild("minwordlen")
        maxentries = xDialog.getChild("maxentries")

        self.assertEqual(get_state_as_dict(enablewordcomplete)["Selected"], "true")
        self.assertEqual(get_state_as_dict(appendspace)["Selected"], "true")
        self.assertEqual(get_state_as_dict(showastip)["Selected"], "false")
        self.assertEqual(get_state_as_dict(whenclosing)["Selected"], "true")

        self.assertEqual(get_state_as_dict(acceptwith)["SelectEntryText"], "Right")
        self.assertEqual(get_state_as_dict(minwordlen)["Text"], "6")
        self.assertEqual(get_state_as_dict(maxentries)["Text"], "100")

        xOKButton = xDialog.getChild("ok")
        self.ui_test.close_dialog_through_button(xOKButton)

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
