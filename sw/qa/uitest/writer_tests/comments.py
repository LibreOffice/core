# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from libreoffice.uno.propertyvalue import mkPropertyValues
#test comments

class Comments(UITestCase):

    def test_comments_features(self):

        xMainDoc = self.ui_test.create_doc_in_start_center("writer")

        xMainWindow = self.xUITest.getTopFocusWindow()

        xwriter_edit = xMainWindow.getChild("writer_edit")
        xwriter_edit.executeAction("TYPE", mkPropertyValues({"TEXT": "Before "}))

        # adding new Comment
        self.xUITest.executeCommand(".uno:InsertAnnotation")

        # wait until the comment is available
        xComment1 = self.ui_test.wait_until_child_is_available('Comment1')

        xEditView1 = xComment1.getChild("editview")
        xEditView1.executeAction("TYPE", mkPropertyValues({"TEXT": "This is the First Comment"}))
        self.assertEqual(get_state_as_dict(xComment1)["Text"], "This is the First Comment" )
        self.assertEqual(get_state_as_dict(xComment1)["Resolved"], "false" )
        self.assertEqual(get_state_as_dict(xComment1)["Author"], "Unknown Author" )
        self.assertEqual(get_state_as_dict(xComment1)["ReadOnly"], "false" )

        xComment1.executeAction("LEAVE", mkPropertyValues({}))

        xwriter_edit.executeAction("TYPE", mkPropertyValues({"TEXT": "After"}))
        xwriter_edit.executeAction("SELECT", mkPropertyValues({"END_POS": "0", "START_POS": "13"}))
        self.assertEqual(get_state_as_dict(xwriter_edit)["SelectedText"], "Before After" )

        # test Resolve Comment
        xComment1.executeAction("RESOLVE", mkPropertyValues({}))
        self.assertEqual(get_state_as_dict(xComment1)["Resolved"], "true" )

        # test Select text from Comment
        xComment1.executeAction("SELECT", mkPropertyValues({"FROM": "0", "TO": "4"}))
        self.assertEqual(get_state_as_dict(xComment1)["SelectedText"], "This" )

        # test Hide then Show Comment
        xComment1.executeAction("HIDE", mkPropertyValues({}))
        self.assertEqual(get_state_as_dict(xComment1)["Visible"], "false" )
        xComment1.executeAction("SHOW", mkPropertyValues({}))
        self.assertEqual(get_state_as_dict(xComment1)["Visible"], "true" )

        # test delete Comment
        xComment1.executeAction("DELETE", mkPropertyValues({}))
        self.assertTrue("Comment1" not in xMainWindow.getChildren())

        self.ui_test.close_doc()

    def test_multi_comments(self):

        xMainDoc = self.ui_test.create_doc_in_start_center("writer")

        xMainWindow = self.xUITest.getTopFocusWindow()

        xwriter_edit = xMainWindow.getChild("writer_edit")

        # adding 3 new Comment
        xwriter_edit.executeAction("TYPE", mkPropertyValues({"TEXT": "Line 1"}))
        self.xUITest.executeCommand(".uno:InsertAnnotation")
        # wait until the comment is available
        xComment1 = self.ui_test.wait_until_child_is_available('Comment1')
        xEditView1 = xComment1.getChild("editview")
        xEditView1.executeAction("TYPE", mkPropertyValues({"TEXT": "First Comment"}))
        xComment1.executeAction("LEAVE", mkPropertyValues({}))
        xwriter_edit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))

        xwriter_edit.executeAction("TYPE", mkPropertyValues({"TEXT": "Line 2"}))
        self.xUITest.executeCommand(".uno:InsertAnnotation")
        # wait until the comment is available
        xComment2 = self.ui_test.wait_until_child_is_available('Comment2')
        xEditView2 = xComment2.getChild("editview")
        xEditView2.executeAction("TYPE", mkPropertyValues({"TEXT": "Second Comment"}))
        xComment2.executeAction("LEAVE", mkPropertyValues({}))
        xwriter_edit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))

        xwriter_edit.executeAction("TYPE", mkPropertyValues({"TEXT": "Line 3"}))
        self.xUITest.executeCommand(".uno:InsertAnnotation")
        # wait until the comment is available
        xComment3 = self.ui_test.wait_until_child_is_available('Comment3')
        xEditView3 = xComment3.getChild("editview")
        xEditView3.executeAction("TYPE", mkPropertyValues({"TEXT": "Third Comment"}))
        xComment3.executeAction("LEAVE", mkPropertyValues({}))
        xwriter_edit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))

        # Check text
        self.assertEqual(get_state_as_dict(xComment1)["Text"], "First Comment" )
        self.assertEqual(get_state_as_dict(xComment2)["Text"], "Second Comment" )
        self.assertEqual(get_state_as_dict(xComment3)["Text"], "Third Comment" )

        xComment2.executeAction("RESOLVE", mkPropertyValues({}))
        xComment3.executeAction("RESOLVE", mkPropertyValues({}))

        # Check Resolved
        self.assertEqual(get_state_as_dict(xComment1)["Resolved"], "false" )
        self.assertEqual(get_state_as_dict(xComment2)["Resolved"], "true" )
        self.assertEqual(get_state_as_dict(xComment3)["Resolved"], "true" )

        # Check ReadOnly
        self.assertEqual(get_state_as_dict(xComment1)["ReadOnly"], "false" )
        self.assertEqual(get_state_as_dict(xComment2)["ReadOnly"], "false" )
        self.assertEqual(get_state_as_dict(xComment3)["ReadOnly"], "false" )

        # Check Select
        xComment1.executeAction("SELECT", mkPropertyValues({"FROM": "0", "TO": "5"}))
        self.assertEqual(get_state_as_dict(xComment1)["SelectedText"], "First" )

        xComment2.executeAction("SELECT", mkPropertyValues({"FROM": "0", "TO": "6"}))
        self.assertEqual(get_state_as_dict(xComment2)["SelectedText"], "Second" )

        xComment3.executeAction("SELECT", mkPropertyValues({"FROM": "0", "TO": "5"}))
        self.assertEqual(get_state_as_dict(xComment3)["SelectedText"], "Third" )

        # Check that they all are Visible
        self.assertEqual(get_state_as_dict(xComment1)["Visible"], "true" )
        self.assertEqual(get_state_as_dict(xComment2)["Visible"], "true" )
        self.assertEqual(get_state_as_dict(xComment3)["Visible"], "true" )

        # Hide all
        xComment1.executeAction("HIDE", mkPropertyValues({}))
        self.assertEqual(get_state_as_dict(xComment1)["Visible"], "false" )
        xComment2.executeAction("HIDE", mkPropertyValues({}))
        self.assertEqual(get_state_as_dict(xComment2)["Visible"], "false" )
        xComment3.executeAction("HIDE", mkPropertyValues({}))
        self.assertEqual(get_state_as_dict(xComment3)["Visible"], "false" )

        # Show comment 2 only
        xComment2.executeAction("SHOW", mkPropertyValues({}))
        self.assertEqual(get_state_as_dict(xComment2)["Visible"], "true" )

        # Then remove the 3 comments
        xComment1.executeAction("DELETE", mkPropertyValues({}))
        xComment2.executeAction("DELETE", mkPropertyValues({}))
        xComment3.executeAction("DELETE", mkPropertyValues({}))
        self.assertTrue("Comment1" not in xMainWindow.getChildren())
        self.assertTrue("Comment2" not in xMainWindow.getChildren())
        self.assertTrue("Comment3" not in xMainWindow.getChildren())

        self.ui_test.close_doc()

# vim: set shiftwidth=4 softtabstop=4 expandtab:

