# -*- Mode: python; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import time

from helper import EventListener

class UITest(object):

    def __init__(self, xUITest, xContext):
        self._xUITest = xUITest
        self._xContext = xContext

    def execute_dialog_through_command(self, command):
        with EventListener(self._xContext, "DialogExecute") as event:
            self._xUITest.executeCommand(command)
            time_ = 0
            while time_ < 30:
                if event.executed:
                    time.sleep(1)
                    return
                time_ += 1
                time.sleep(1)

        # report a failure here
        print("failure execute modal dialog")

    def execute_modeless_dialog_through_command(self, command):
        with EventListener(self._xContext, "ModelessDialogVisible") as event:
            self._xUITest.executeCommand(command)
            time_ = 0
            while time_ < 30:
                if event.executed:
                    time.sleep(1)
                    return
                time_ += 1
                time.sleep(1)

        # report a failure here
        print("failure execute modeless dialog")

    def create_doc_in_start_center(self, app):
        xStartCenter = self._xUITest.getTopFocusWindow()
        xBtn = xStartCenter.getChild(app + "_all")
        with EventListener(self._xContext, "OnNew") as event:
            xBtn.executeAction("CLICK", tuple())
            time_ = 0
            while time_ < 30:
                if event.executed:
                    return
                time_ += 1
                time.sleep(1)

        print("failure doc in start center")

        # report a failure here

    def close_doc(self):
        # also need to handle "OnViewClosed" event
        with EventListener(self._xContext, "DialogExecute") as event:
            self._xUITest.executeCommand(".uno.CloseDoc")
            time_ = 0
            while time_ < 30:
                if event.executed:
                    xCloseDlg = self._xUITest.getTopFocusWindow()
                    xNoBtn = xCloseDlg.getChild("discard")
                    xNoBtn.executeAction("CLICK", tuple())
                    return

                time_ += 1
                time.sleep(1)

# vim:set shiftwidth=4 softtabstop=4 expandtab: */
