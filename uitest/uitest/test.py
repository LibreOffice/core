# -*- Mode: python; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import time
from uitest.config import DEFAULT_SLEEP

from libreoffice.uno.eventlistener import EventListener

class DialogNotExecutedException(Exception):
    def __init__(self, command):
        self.command = command

    def __str__(self):
        return "Dialog not executed for: " + self.command

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
                    time.sleep(DEFAULT_SLEEP)
                    return
                time_ += DEFAULT_SLEEP
                time.sleep(DEFAULT_SLEEP)

        raise DialogNotExecutedException(command)

    def execute_modeless_dialog_through_command(self, command):
        with EventListener(self._xContext, "ModelessDialogVisible") as event:
            self._xUITest.executeCommand(command)
            time_ = 0
            while time_ < 30:
                if event.executed:
                    time.sleep(DEFAULT_SLEEP)
                    return
                time_ += DEFAULT_SLEEP
                time.sleep(DEFAULT_SLEEP)

        raise DialogNotExecutedException(command)

    def execute_dialog_through_action(self, ui_object, action, parameters = None):
        if parameters is None:
            parameters = tuple()

        with EventListener(self._xContext, "DialogExecute") as event:
            ui_object.executeAction(action, parameters)
            time_ = 0
            while time_ < 30:
                if event.executed:
                    time.sleep(DEFAULT_SLEEP)
                    return
                time_ += DEFAULT_SLEEP
                time.sleep(DEFAULT_SLEEP)
        raise DialogNotExecutedException(command)

    def create_doc_in_start_center(self, app):
        xStartCenter = self._xUITest.getTopFocusWindow()
        xBtn = xStartCenter.getChild(app + "_all")
        with EventListener(self._xContext, "OnNew") as event:
            xBtn.executeAction("CLICK", tuple())
            time_ = 0
            while time_ < 30:
                if event.executed:
                    return
                time_ += DEFAULT_SLEEP
                time.sleep(DEFAULT_SLEEP)

        print("failure doc in start center")

        # report a failure here

    def close_doc(self):
        with EventListener(self._xContext, ["DialogExecute", "OnViewClosed"] ) as event:
            self._xUITest.executeCommand(".uno:CloseDoc")
            time_ = 0
            while time_ < 30:
                if event.hasExecuted("DialogExecute"):
                    xCloseDlg = self._xUITest.getTopFocusWindow()
                    xNoBtn = xCloseDlg.getChild("discard")
                    xNoBtn.executeAction("CLICK", tuple())
                    return
                elif event.hasExecuted("OnViewClosed"):
                    return

                time_ += DEFAULT_SLEEP
                time.sleep(DEFAULT_SLEEP)

# vim:set shiftwidth=4 softtabstop=4 expandtab: */
