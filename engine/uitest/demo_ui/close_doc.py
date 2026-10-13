# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from libreoffice.uno.eventlistener import EventListener
from uitest.framework import UITestCase
from com.sun.star.lang import DisposedException


class ModifyDuringClose(EventListener):
    def documentEventOccured(self, event):
        if event.EventName == "OnPrepareUnload":
            event.Source.setModified(True)
        super().documentEventOccured(event)


class CloseDocTest(UITestCase):
    def test_close_unmodified(self):
        self.ui_test.set_use_dispose(False)
        with self.ui_test.create_doc_in_start_center("math") as document:
            self.assertFalse(document.isModified())
        with self.assertRaises(DisposedException):
            document.isModified()
        self.xUITest.getTopFocusWindow().getChild("math_all")

    def test_close_modified(self):
        self.ui_test.set_use_dispose(False)
        with self.ui_test.create_doc_in_start_center("math") as document:
            document.Formula = "backepsilon"
            self.assertTrue(document.isModified())
        with self.assertRaises(DisposedException):
            document.isModified()
        self.xUITest.getTopFocusWindow().getChild("math_all")

    def test_close_modified_during_prepare(self):
        self.ui_test.set_use_dispose(False)
        # Force the ordering of a deferred edit: the helper sees an unmodified model, but close
        # preparation modifies it before SfxObjectShell checks whether to show the save prompt.
        with ModifyDuringClose(self.xContext, "OnPrepareUnload") as event:
            with self.ui_test.create_doc_in_start_center("math") as document:
                self.assertFalse(document.isModified())
            self.assertTrue(event.executed)
        with self.assertRaises(DisposedException):
            document.isModified()
        self.xUITest.getTopFocusWindow().getChild("math_all")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
