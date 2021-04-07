# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

from uitest.framework import UITestCase
from libreoffice.uno.propertyvalue import mkPropertyValues
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import select_pos

class autocorrectOptions(UITestCase):

   def test_autocorrect_options_calc(self):
        self.ui_test.create_doc_in_start_center("calc")
        xCalcDoc = self.xUITest.getTopFocusWindow()
        gridwin = xCalcDoc.getChild("grid_window")
        document = self.ui_test.get_component()

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

        self.assertTrue(int(nrRows) > 0)

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

        nrRowsAbb = get_state_as_dict(abbrevlist)["VisibleCount"]

        self.assertTrue(int(nrRowsAbb) > 0)

        abbrev.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        abbrev.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        abbrev.executeAction("TYPE", mkPropertyValues({"TEXT":"qqqqq"}))
        newabbrev.executeAction("CLICK", tuple())
        nrRowsAbbNew = get_state_as_dict(abbrevlist)["VisibleCount"]
        nrRowsAbbDiff = int(nrRowsAbbNew) - int(nrRowsAbb)
        self.assertEqual(nrRowsAbbDiff, 1)  #we have +1 rule
        delabbrev.executeAction("CLICK", tuple())
        self.assertEqual(get_state_as_dict(abbrevlist)["VisibleCount"], nrRowsAbb)   #we have default nr of rules

        #words with two initial capitals
        double = xDialog.getChild("double")
        newdouble = xDialog.getChild("newdouble")
        deldouble = xDialog.getChild("deldouble")
        doublelist = xDialog.getChild("doublelist")

        nrRowsDouble = get_state_as_dict(doublelist)["VisibleCount"]

        self.assertTrue(int(nrRowsDouble) > 0)

        double.executeAction("TYPE", mkPropertyValues({"KEYCODE":"CTRL+A"}))
        double.executeAction("TYPE", mkPropertyValues({"KEYCODE":"BACKSPACE"}))
        double.executeAction("TYPE", mkPropertyValues({"TEXT":"QQqqq"}))
        newdouble.executeAction("CLICK", tuple())
        nrRowsDoubleNew = get_state_as_dict(doublelist)["VisibleCount"]
        nrRowsDoubleDiff = int(nrRowsDoubleNew) - int(nrRowsDouble) #convert string and
        self.assertEqual(nrRowsDoubleDiff, 1)  #we have +1 rule
        deldouble.executeAction("CLICK", tuple())
        self.assertEqual(get_state_as_dict(doublelist)["VisibleCount"], nrRowsDouble)   #we have default nr of rules

        xCancelButton = xDialog.getChild("cancel")
        xCancelButton.executeAction("CLICK", tuple())

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
