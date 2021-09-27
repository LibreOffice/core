# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict, get_url_for_data_file

import time

class tdf144578(UITestCase):

    def test_tdf144578(self):
        with self.ui_test.load_file(get_url_for_data_file("tdf144578.odt")) as writer_doc:
            with self.ui_test.execute_dialog_through_command(".uno:ChapterNumberingDialog") as xDialog:
                # Select level "1"
                xLevelsTree = xDialog.getChild("level")
                xLevel = xLevelsTree.getChild("0")
                xLevel.executeAction("SELECT", tuple())
                # Check value for show upper levels
                xSubLevels = xDialog.getChild("sublevelsnf")
                self.assertEqual(get_state_as_dict(xSubLevels)["Text"], "1")

                # Select level "2"
                xLevel = xLevelsTree.getChild("1")
                xLevel.executeAction("SELECT", tuple())
                # Check value for show upper levels
                xSubLevels = xDialog.getChild("sublevelsnf")
                self.assertEqual(get_state_as_dict(xSubLevels)["Text"], "2")

                # Select level "3"
                xLevel = xLevelsTree.getChild("2")
                xLevel.executeAction("SELECT", tuple())
                # Check value for show upper levels
                xSubLevels = xDialog.getChild("sublevelsnf")
                self.assertEqual(get_state_as_dict(xSubLevels)["Text"], "3")

                # Select level "3"
                xLevel = xLevelsTree.getChild("3")
                xLevel.executeAction("SELECT", tuple())
                # Check value for show upper levels
                xSubLevels = xDialog.getChild("sublevelsnf")
                self.assertEqual(get_state_as_dict(xSubLevels)["Text"], "1")

            # And also verify label strings in outlines
            Paragraphs = []
            ParagraphEnum = writer_doc.Text.createEnumeration()
            while ParagraphEnum.hasMoreElements():
                Para = ParagraphEnum.nextElement()
                Paragraphs.append(Para)

            self.assertEqual(Paragraphs[0].getPropertyValue("ListLabelString"), "I.")
            self.assertEqual(Paragraphs[2].getPropertyValue("ListLabelString"), "II.")
            self.assertEqual(Paragraphs[4].getPropertyValue("ListLabelString"), "II.A.")
            self.assertEqual(Paragraphs[6].getPropertyValue("ListLabelString"), "II.B.")
            self.assertEqual(Paragraphs[8].getPropertyValue("ListLabelString"), "III.")
            self.assertEqual(Paragraphs[10].getPropertyValue("ListLabelString"), "III.A.")
            self.assertEqual(Paragraphs[11].getPropertyValue("ListLabelString"), "III.A.1.")
            self.assertEqual(Paragraphs[13].getPropertyValue("ListLabelString"), "III.A.2.")
            self.assertEqual(Paragraphs[15].getPropertyValue("ListLabelString"), "III.B.")
            self.assertEqual(Paragraphs[17].getPropertyValue("ListLabelString"), "III.C.")
            self.assertEqual(Paragraphs[19].getPropertyValue("ListLabelString"), "III.C.1.")
            self.assertEqual(Paragraphs[21].getPropertyValue("ListLabelString"), "III.C.2.")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
