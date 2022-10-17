# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest.framework import UITestCase


class Test(UITestCase):
    def testContentControlAliasButton(self):
        with self.ui_test.create_doc_in_start_center("writer") as xComponent:
            # Given a document with a content control that has a non-empty alias:
            self.xUITest.executeCommand(".uno:InsertContentControl")
            paragraphs = xComponent.Text.createEnumeration()
            paragraph = paragraphs.nextElement()
            portions = paragraph.createEnumeration()
            portion = portions.nextElement()
            contentControl = portion.ContentControl
            contentControl.Alias = "myalias"

            # When entering that content control with the cursor:
            xCursor = xComponent.getCurrentController().getViewCursor()
            xCursor.gotoStart(False)
            xCursor.goRight(1, False)

            # Then make sure that the alias button shows up:
            xWindow = self.xUITest.getTopFocusWindow()
            # Without the accompanying fix in place, this test would have failed, SwEditWin didn't
            # have an alias button child.
            self.assertIn("ContentControlAliasButton", xWindow.getChildren())

# vim: set shiftwidth=4 softtabstop=4 expandtab:
