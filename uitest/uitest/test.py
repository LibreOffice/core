# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import time
import threading
from contextlib import contextmanager
from uitest.config import DEFAULT_SLEEP
from uitest.config import MAX_WAIT
from uitest.uihelper.common import get_state_as_dict

from com.sun.star.uno import RuntimeException

from libreoffice.uno.eventlistener import EventListener

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

    def wait_for_top_focus_window(self, id):
        while True:
            win = self._xUITest.getTopFocusWindow()
            if get_state_as_dict(win)['ID'] == id:
                return win
            time.sleep(DEFAULT_SLEEP)

    def wait_until_child_is_available(self, childName):
        time_ = 0

        while time_ < MAX_WAIT:
            xDialog = self._xUITest.getTopFocusWindow()
            if childName in xDialog.getChildren():
                return xDialog.getChild(childName)
            else:
                time_ += DEFAULT_SLEEP
                time.sleep(DEFAULT_SLEEP)

        raise Exception("Child not found: " + childName)

    def wait_until_property_is_updated(self, element, propertyName, value):
        time_ = 0
        while time_ < MAX_WAIT:
            if get_state_as_dict(element)[propertyName] == value:
                return
            else:
                time_ += DEFAULT_SLEEP
                time.sleep(DEFAULT_SLEEP)

        raise Exception("Property not updated: " + childName)

    @contextmanager
    def wait_until_component_loaded(self, eventName="OnLoad"):
        with EventListener(self._xContext, eventName) as event:
            yield
            time_ = 0
            while time_ < MAX_WAIT:
                if event.executed:
                    frames = self.get_frames()
                    if len(frames) == 1:
                        self.get_desktop().setActiveFrame(frames[0])
                    time.sleep(DEFAULT_SLEEP)
                    return
                time_ += DEFAULT_SLEEP
                time.sleep(DEFAULT_SLEEP)

        raise Exception("Component not loaded")

    # Calls UITest.close_doc at exit
    @contextmanager
    def load_file(self, url):
        try:
            with self.wait_until_component_loaded():
                yield self.get_desktop().loadComponentFromURL(url, "_default", 0, tuple())
        finally:
            self.close_doc()

    # Calls UITest.close_doc at exit
    @contextmanager
    def load_empty_file(self, app):
        try:
            with self.wait_until_component_loaded("OnNew"):
                yield self.get_desktop().loadComponentFromURL("private:factory/s" + app, "_blank", 0, tuple())
        finally:
            self.close_doc()

    # Calls UITest.close_dialog_through_button at exit
    @contextmanager
    def execute_dialog_through_command(self, command, printNames=False, close_button = "ok"):
        with EventListener(self._xContext, "DialogExecute", printNames=printNames) as event:
            if not self._xUITest.executeDialog(command):
                raise Exception("Dialog not executed for: " + command)
            while True:
                if event.executed:
                    xDialog = self._xUITest.getTopFocusWindow()
                    try:
                        yield xDialog
                    finally:
                        if close_button:
                            self.close_dialog_through_button(xDialog.getChild(close_button))
                    return
                time.sleep(DEFAULT_SLEEP)

    def execute_modeless_dialog_through_command(self, command, printNames=False):
        with EventListener(self._xContext, "ModelessDialogVisible", printNames = printNames) as event:
            if not self._xUITest.executeCommand(command):
                raise Exception("Dialog not executed for: " + command)
            time_ = 0
            while time_ < MAX_WAIT:
                if event.executed:
                    time.sleep(DEFAULT_SLEEP)
                    return
                time_ += DEFAULT_SLEEP
                time.sleep(DEFAULT_SLEEP)

        raise Exception("Dialog not executed for: " + command)

    # Calls UITest.close_dialog_through_button at exit
    @contextmanager
    def execute_dialog_through_action(self, ui_object, action, parameters = None, event_name = "DialogExecute", close_button = "ok"):
        if parameters is None:
            parameters = tuple()

        with EventListener(self._xContext, event_name) as event:
            ui_object.executeAction(action, parameters)
            time_ = 0
            while time_ < MAX_WAIT:
                if event.executed:
                    xDialog = self._xUITest.getTopFocusWindow()
                    try:
                        yield xDialog
                    finally:
                        self.close_dialog_through_button(xDialog.getChild(close_button))
                    return
                time_ += DEFAULT_SLEEP
                time.sleep(DEFAULT_SLEEP)
        raise Exception("Dialog not executed for: " + action)

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

    # Calls UITest.close_doc at exit
    @contextmanager
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
                    component = self.get_component()
                    try:
                        yield component
                    finally:
                        self.close_doc()
                    return
                time_ += DEFAULT_SLEEP
                time.sleep(DEFAULT_SLEEP)

        raise Exception("Failure doc in start center")

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

    @contextmanager
    def execute_blocking_action(
            self, action, args=(), close_button="ok", printNames=False, id=None):
        """Executes an action which blocks while a dialog is shown.

        Click a button or perform some other action on the dialog when it
        is shown.

        Args:
            action(callable): Will be called to show a dialog, and is expected
                to block while the dialog is shown.
            close_button(str): The name of a button which will be clicked to close
                the dialog. if it's empty, the dialog won't be closed from here.
                This is useful when consecutive dialogs are open one after the other.
            args(tuple, optional): The arguments to be passed to `action`
            printNames: print all received event names
        """

        thread = threading.Thread(target=action, args=args)
        with EventListener(self._xContext, ["DialogExecute", "ModelessDialogExecute", "ModelessDialogVisible"], printNames=printNames) as event:
            thread.start()
            time_ = 0
            # we are not necessarily opening a dialog, so wait much longer
            while time_ < 10 * MAX_WAIT:
                if event.executed:
                    if id:
                        xDialog = self.wait_for_top_focus_window(id)
                    else:
                        xDialog = self._xUITest.getTopFocusWindow()
                    try:
                        yield xDialog
                    finally:
                        if close_button:
                            self.close_dialog_through_button(xDialog.getChild(close_button))
                        thread.join()
                    return
                time_ += DEFAULT_SLEEP
                time.sleep(DEFAULT_SLEEP)
        raise Exception("Did not execute a dialog for a blocking action")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
