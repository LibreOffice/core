# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import time
import threading
from uitest.config import DEFAULT_SLEEP
from uitest.config import MAX_WAIT
from uitest.uihelper.common import get_state_as_dict

from com.sun.star.uno import RuntimeException

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

    def wait_until_child_is_available(self, childName):
        time_ = 0
        xChild = None

        while time_ < MAX_WAIT:
            xDialog = self._xUITest.getTopFocusWindow()
            if childName in xDialog.getChildren():
                xChild = xDialog.getChild(childName)
                break
            else:
                time_ += DEFAULT_SLEEP
                time.sleep(DEFAULT_SLEEP)

        return xChild

    def wait_until_property_is_updated(self, element, propertyName, value):
        time_ = 0
        while time_ < MAX_WAIT:
            if get_state_as_dict(element)[propertyName] == value:
                break
            else:
                time_ += DEFAULT_SLEEP
                time.sleep(DEFAULT_SLEEP)

    def load_file(self, url):
        desktop = self.get_desktop()
        with EventListener(self._xContext, "OnLoad") as event:
            component = desktop.loadComponentFromURL(url, "_default", 0, tuple())
            time_ = 0
            while time_ < MAX_WAIT:
                if event.executed:
                    frames = self.get_frames()
                    if len(frames) == 1:
                        self.get_desktop().setActiveFrame(frames[0])
                    time.sleep(DEFAULT_SLEEP)
                    return component
                time_ += DEFAULT_SLEEP
                time.sleep(DEFAULT_SLEEP)

    def execute_dialog_through_command(self, command, printNames=False):
        with EventListener(self._xContext, "DialogExecute", printNames=printNames) as event:
            if not self._xUITest.executeDialog(command):
                raise DialogNotExecutedException(command)
            while True:
                if event.executed:
                    time.sleep(DEFAULT_SLEEP)
                    return
                time.sleep(DEFAULT_SLEEP)

    def execute_modeless_dialog_through_command(self, command, printNames=False):
        with EventListener(self._xContext, "ModelessDialogVisible", printNames = printNames) as event:
            if not self._xUITest.executeCommand(command):
                raise DialogNotExecutedException(command)
            time_ = 0
            while time_ < MAX_WAIT:
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
            while time_ < MAX_WAIT:
                if event.executed:
                    time.sleep(DEFAULT_SLEEP)
                    return
                time_ += DEFAULT_SLEEP
                time.sleep(DEFAULT_SLEEP)
        raise DialogNotExecutedException(action)

    def _handle_crash_reporter(self):
        xCrashReportDlg = self._xUITest.getTopFocusWindow()
        state = get_state_as_dict(xCrashReportDlg)
        print(state)
        if state['ID'] != "CrashReportDialog":
            return False
        print("found a crash reporter")
        xCancelBtn = xCrashReportDlg.getChild("btn_cancel")
        self.close_dialog_through_button(xCancelBtn)
        return True

    def create_doc_in_start_center(self, app):
        xStartCenter = self._xUITest.getTopFocusWindow()
        try:
            xBtn = xStartCenter.getChild(app + "_all")
        except RuntimeException:
            if self._handle_crash_reporter():
                xStartCenter = self._xUITest.getTopFocusWindow()
                xBtn = xStartCenter.getChild(app + "_all")
            else:
                raise

        with EventListener(self._xContext, "OnNew") as event:
            xBtn.executeAction("CLICK", tuple())
            time_ = 0
            while time_ < MAX_WAIT:
                if event.executed:
                    frames = self.get_frames()
                    self.get_desktop().setActiveFrame(frames[0])
                    return
                time_ += DEFAULT_SLEEP
                time.sleep(DEFAULT_SLEEP)

        print("failure doc in start center")

        # report a failure here

    def close_dialog_through_button(self, button):
        with EventListener(self._xContext, "DialogClosed" ) as event:
            button.executeAction("CLICK", tuple())
            while True:
                if event.executed:
                    time.sleep(DEFAULT_SLEEP)
                    return
                time.sleep(DEFAULT_SLEEP)

    def close_doc(self):
        desktop = self.get_desktop()
        active_frame = desktop.getActiveFrame()
        if not active_frame:
            print("close_doc: no active frame")
            return
        component = active_frame.getController().getModel()
        if not component:
            print("close_doc: active frame has no component")
            return
        component.dispose()
        frames = desktop.getFrames()
        if frames:
            frames[0].activate()

    def execute_blocking_action(self, action, dialog_element=None,
            args=(), dialog_handler=None, printNames=False):
        """Executes an action which blocks while a dialog is shown.

        Click a button or perform some other action on the dialog when it
        is shown.

        Args:
            action(callable): Will be called to show a dialog, and is expected
                to block while the dialog is shown.
            dialog_element(str, optional): The name of a button on the dialog
                which will be clicked when the dialog is shown.
            args(tuple, optional): The arguments to be passed to `action`
            dialog_handler(callable, optional): Will be called when the dialog
                is shown, with the dialog object passed as a parameter.
            printNames: print all received event names
        """

        thread = threading.Thread(target=action, args=args)
        with EventListener(self._xContext, ["DialogExecute", "ModelessDialogExecute", "ModelessDialogVisible"], printNames=printNames) as event:
            thread.start()
            time_ = 0
            # we are not necessarily opening a dialog, so wait much longer
            while time_ < 10 * MAX_WAIT:
                if event.executed:
                    xDlg = self._xUITest.getTopFocusWindow()
                    if dialog_element:
                        xUIElement = xDlg.getChild(dialog_element)
                        xUIElement.executeAction("CLICK", tuple())
                    if dialog_handler:
                        dialog_handler(xDlg)
                    thread.join()
                    return

                time_ += DEFAULT_SLEEP
                time.sleep(DEFAULT_SLEEP)
        raise DialogNotExecutedException("did not execute a dialog for a blocking action")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
