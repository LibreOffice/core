# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import time
import threading
from uitest.config import DEFAULT_SLEEP

from libreoffice.uno.eventlistener import EventListener

class DialogNotExecutedException(Exception):
    def __init__(self, command):
        self.command = command

    def __str__(self):
        return "Dialog not executed for: " + self.command

class DialogNotClosedException(Exception):

    def __str__(self):
        return "Dialog was not closed"

class UITest(object):

    def __init__(self, xUITest, xContext):
        self._xUITest = xUITest
        self._xContext = xContext
        self._desktop = None

    def get_desktop(self):
        if self._desktop:
            return self._desktop

        self._desktop = self._xContext.ServiceManager.createInstanceWithContext("com.sun.star.frame.Desktop", self._xContext)
        return self._desktop

    def get_frames(self):
        desktop = self.get_desktop()
        frames = desktop.getFrames()
        return frames

    def get_component(self):
        desktop = self.get_desktop()
        components = desktop.getComponents()
        for component in components:
            if component is not None:
                return component

    def load_file(self, url):
        desktop = self.get_desktop()
        with EventListener(self._xContext, "OnLoad") as event:
            component = desktop.loadComponentFromURL(url, "_default", 0, tuple())
            time_ = 0
            while time_ < 30:
                if event.executed:
                    frames = self.get_frames()
                    if len(frames) == 1:
                        self.get_desktop().setActiveFrame(frames[0])
                    time.sleep(DEFAULT_SLEEP)
                    return component
                time_ += DEFAULT_SLEEP
                time.sleep(DEFAULT_SLEEP)

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

    def execute_dialog_through_action(self, ui_object, action, parameters = None, event_name = "DialogExecute"):
        if parameters is None:
            parameters = tuple()

        with EventListener(self._xContext, event_name) as event:
            ui_object.executeAction(action, parameters)
            time_ = 0
            while time_ < 30:
                if event.executed:
                    time.sleep(DEFAULT_SLEEP)
                    return
                time_ += DEFAULT_SLEEP
                time.sleep(DEFAULT_SLEEP)
        raise DialogNotExecutedException(action)

    def create_doc_in_start_center(self, app):
        xStartCenter = self._xUITest.getTopFocusWindow()
        xBtn = xStartCenter.getChild(app + "_all")
        with EventListener(self._xContext, "OnNew") as event:
            xBtn.executeAction("CLICK", tuple())
            time_ = 0
            while time_ < 30:
                if event.executed:
                    frames = self.get_frames()
                    self.get_desktop().setActiveFrame(frames[0])
                    print(len(frames))
                    return
                time_ += DEFAULT_SLEEP
                time.sleep(DEFAULT_SLEEP)

        print("failure doc in start center")

        # report a failure here

    def close_dialog_through_button(self, button):
        with EventListener(self._xContext, "DialogClosed" ) as event:
            button.executeAction("CLICK", tuple())
            time_ = 0
            while time_ < 30:
                if event.executed:
                    time.sleep(DEFAULT_SLEEP)
                    return
                time_ += DEFAULT_SLEEP
                time.sleep(DEFAULT_SLEEP)
        raise DialogNotClosedException()

    def close_doc(self):
        with EventListener(self._xContext, ["DialogExecute", "OnViewClosed"] ) as event:
            self._xUITest.executeCommand(".uno:CloseDoc")
            time_ = 0
            while time_ < 30:
                if event.hasExecuted("DialogExecute"):
                    xCloseDlg = self._xUITest.getTopFocusWindow()
                    xNoBtn = xCloseDlg.getChild("discard")
                    self.close_dialog_through_button(xNoBtn);
                    return
                elif event.hasExecuted("OnViewClosed"):
                    return

                time_ += DEFAULT_SLEEP
                time.sleep(DEFAULT_SLEEP)

    def execute_blocking_action(self, action, dialog_element, args = ()):
        thread = threading.Thread(target=action, args=args)
        with EventListener(self._xContext, ["DialogExecute", "ModelessDialogExecute"]) as event:
            thread.start()
            time_ = 0
            while time_ < 30:
                if event.executed:
                    xDlg = self._xUITest.getTopFocusWindow()
                    xUIElement = xDlg.getChild(dialog_element)
                    xUIElement.executeAction("CLICK", tuple())
                    thread.join()
                    return

                time_ += DEFAULT_SLEEP
                time.sleep(DEFAULT_SLEEP)
        raise DialogNotExecutedException("did not execute a dialog for a blocking action")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
