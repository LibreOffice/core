# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import time
import threading
import tempfile
from contextlib import contextmanager
from uitest.uihelper.common import get_state_as_dict, select_by_text

from com.sun.star.uno import RuntimeException

from libreoffice.uno.eventlistener import EventListener
from libreoffice.uno.propertyvalue import mkPropertyValues

DEFAULT_SLEEP = 0.1

class UITest(object):

    def __init__(self, xUITest, xContext):
        self._xUITest = xUITest
        self._xContext = xContext
        self._desktop = None
        self.use_dispose = True

    def set_use_dispose(self, use_dispose):
        self.use_dispose = use_dispose

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

    def get_default_sleep(self):
        return DEFAULT_SLEEP

    def wait_for_top_focus_window(self, id):
        while True:
            win = self._xUITest.getTopFocusWindow()
            if get_state_as_dict(win)['ID'] == id:
                return win
            time.sleep(DEFAULT_SLEEP)

    def wait_until_child_is_available(self, childName):
        while True:
            xDialog = self._xUITest.getTopFocusWindow()
            if childName in xDialog.getChildren():
                return xDialog.getChild(childName)
            else:
                time.sleep(DEFAULT_SLEEP)

    def wait_until_property_is_updated(self, element, propertyName, value):
        while True:
            if get_state_as_dict(element)[propertyName] == value:
                return
            else:
                time.sleep(DEFAULT_SLEEP)

    @contextmanager
    def wait_until_component_loaded(self):
        with EventListener(self._xContext, "OnLoad") as event:
            yield
            while True:
                if event.executed:
                    frames = self.get_frames()
                    if len(frames) == 1:
                        self.get_desktop().setActiveFrame(frames[0])
                    time.sleep(DEFAULT_SLEEP)
                    return
                time.sleep(DEFAULT_SLEEP)

    def load_component_from_url(self, url, eventName="OnLoad", load_props=()):
        with EventListener(self._xContext, eventName) as event:
            component =  self.get_desktop().loadComponentFromURL(url, "_default", 0, load_props)
            while True:
                if event.executed:
                    frames = self.get_frames()
                    #activate the newest frame
                    self.get_desktop().setActiveFrame(frames[-1])
                    return component
                time.sleep(DEFAULT_SLEEP)

    # Calls UITest.close_doc at exit
    @contextmanager
    def load_file(self, url, load_props=()):
        try:
            yield self.load_component_from_url(url, "OnLoad", load_props)
        finally:
            self.close_doc()

    # Resets the setting to the old value at exit
    @contextmanager
    def set_config(self, path, new_value):
        xChanges = self._xContext.ServiceManager.createInstanceWithArgumentsAndContext('com.sun.star.configuration.ReadWriteAccess', ("",), self._xContext)
        try:
            old_value = xChanges.getByHierarchicalName(path)
            xChanges.replaceByHierarchicalName(path, new_value)
            xChanges.commitChanges()
            yield
        finally:
            xChanges.replaceByHierarchicalName(path, old_value)
            xChanges.commitChanges()

    # Calls UITest.close_doc at exit
    @contextmanager
    def load_empty_file(self, app):
        try:
            yield self.load_component_from_url("private:factory/s" + app, "OnNew")
        finally:
            self.close_doc()

    def wait_and_yield_dialog(self, event, parent, close_button):
        while not event.executed:
            time.sleep(DEFAULT_SLEEP)
        dialog = self._xUITest.getTopFocusWindow()
        if parent.equals(dialog):
            raise Exception("executing the action did not open the dialog")
        try:
            yield dialog
        except:
            if not close_button:
                if 'cancel' in dialog.getChildren():
                    self.close_dialog_through_button(dialog.getChild("cancel"), dialog)
            raise
        finally:
            if close_button:
                self.close_dialog_through_button(dialog.getChild(close_button), dialog)

    # Calls UITest.close_dialog_through_button at exit
    @contextmanager
    def execute_dialog_through_command(self, command, printNames=False, close_button = "ok", eventName = "DialogExecute"):
        with EventListener(self._xContext, eventName, printNames=printNames) as event:
            xDialogParent = self._xUITest.getTopFocusWindow()
            if not self._xUITest.executeDialog(command):
                raise Exception("Dialog not executed for: " + command)
            yield from self.wait_and_yield_dialog(event, xDialogParent, close_button)

    @contextmanager
    def execute_modeless_dialog_through_command(self, command, printNames=False, close_button = "ok"):
        with self.execute_dialog_through_command(command, printNames, close_button, "ModelessDialogVisible") as xDialog:
            yield xDialog

    # Calls UITest.close_dialog_through_button at exit
    @contextmanager
    def execute_dialog_through_action(self, ui_object, action, parameters = None, event_name = "DialogExecute", close_button = "ok"):
        if parameters is None:
            parameters = tuple()

        xDialogParent = self._xUITest.getTopFocusWindow()
        with EventListener(self._xContext, event_name) as event:
            ui_object.executeAction(action, parameters)
            yield from self.wait_and_yield_dialog(event, xDialogParent, close_button)

    # Executes a command and waits for a subcomponent event to be emitted. The frame from the event
    # will be yielded. If close_win is True then .uno:CloseWin will be called at exit. This can be
    # used for commands that open a new window for the same document.
    @contextmanager
    def open_subcomponent_through_command(self, command, printNames=False, close_win=True):
        with EventListener(self._xContext, "OnSubComponentOpened", printNames=printNames) as event:
            self._xUITest.executeCommand(command)
            while not event.executed:
                time.sleep(DEFAULT_SLEEP)
            frame = event.supplements[0]

        try:
            yield frame
        finally:
            if close_win:
                try:
                    modified = frame.getController().isModified()
                except AttributeError:
                    modified = False
                if modified:
                    # Close the window and answer no when it asks if we want to save
                    with self.execute_blocking_action(self._xUITest.executeCommandForProvider,
                                                      args=(".uno:CloseWin", frame),
                                                      close_button="no"):
                        pass
                else:
                    self._xUITest.executeCommandForProvider(".uno:CloseWin", frame)
                # Closing the window will happen asynchronously on the main thread so let’s wait
                # until the close actually completes.
                xToolkit = self._xContext.ServiceManager.createInstance('com.sun.star.awt.Toolkit')
                xToolkit.waitUntilAllIdlesDispatched()

    # Calls UITest.close_doc at exit
    @contextmanager
    def create_doc_in_start_center(self, app):
        xStartCenter = self._xUITest.getTopFocusWindow()
        try:
            xBtn = xStartCenter.getChild(app + "_all")
        except RuntimeException:
            raise

        with EventListener(self._xContext, "OnNew") as event:
            xBtn.executeAction("CLICK", tuple())
            while True:
                if event.executed:
                    frames = self.get_frames()
                    self.get_desktop().setActiveFrame(frames[0])
                    component = self.get_component()
                    try:
                        yield component
                    finally:
                        self.close_doc()
                    return
                time.sleep(DEFAULT_SLEEP)

    # Creates an empty HSQLDB database with a temporary file name. On exit UITest.close_doc is
    # called and the temporary file is deleted.
    @contextmanager
    def create_db_in_start_center(self):
        # We have to give a filename to the database so let’s create a temporary one
        with tempfile.NamedTemporaryFile(suffix=".odb") as temp:
            with EventListener(self._xContext, "OnNew") as event:
                xStartCenter = self._xUITest.getTopFocusWindow()
                xBtn = xStartCenter.getChild("database_all")
                with self.execute_dialog_through_action(xBtn, "CLICK",
                                                        close_button=None) as xDialog:
                    xDialog.getChild("createDatabase").executeAction("CLICK", tuple())
                    select_by_text(xDialog.getChild("embeddeddbList"), "HSQLDB Embedded")

                    xFinish = xDialog.getChild("finish")

                    # Clicking finish will open a blocking dialog to set the filename
                    with self.execute_blocking_action(xFinish.executeAction,
                                                      ("CLICK", tuple()),
                                                      close_button=None) as xFileDialog:
                        xFileName = xFileDialog.getChild("file_name")
                        xFileName.executeAction("SET", mkPropertyValues({"TEXT": temp.name}))

                        xOpen = xFileDialog.getChild("open")

                        # Clicking the open button will trigger another blocking dialog because the
                        # file already exists and we have to confirm overwriting it
                        with self.execute_blocking_action(xOpen.executeAction,
                                                          ("CLICK", tuple()),
                                                          close_button="yes"):
                            pass

                while not event.executed:
                    time.sleep(DEFAULT_SLEEP)

            frames = self.get_frames()
            self.get_desktop().setActiveFrame(frames[0])
            component = self.get_component()
            try:
                yield component
            finally:
                self.close_doc()

    def close_dialog_through_button(self, button, dialog=None):
        if dialog is None:
            dialog = self._xUITest.getTopFocusWindow()
        if isinstance(button, str):
            button = dialog.getChild(button)
        with EventListener(self._xContext, "DialogClosed" ) as event:
            button.executeAction("CLICK", tuple())
            while True:
                if event.executed:
                    time.sleep(DEFAULT_SLEEP)
                    break
                time.sleep(DEFAULT_SLEEP)
        parent = self._xUITest.getTopFocusWindow()
        if parent.equals(dialog):
            raise Exception("executing the action did not close the dialog")

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
        if self.use_dispose:
            component.dispose()
        else:
            if component.isModified():
                with self.execute_dialog_through_command('.uno:CloseDoc', close_button="discard"):
                    pass
                # execute_dialog_through_command will end up dispatching the command asynchronously
                # on the main thread. The Base code seems to have a few race conditions so to avoid
                # that let’s wait to make sure the close completes before continuing
                xToolkit = self._xContext.ServiceManager.createInstance('com.sun.star.awt.Toolkit')
                xToolkit.waitUntilAllIdlesDispatched()
            else:
                self._xUITest.executeCommand(".uno:CloseDoc")
        frames = desktop.getFrames()
        if frames:
            frames[0].activate()

    @contextmanager
    def execute_blocking_action(self, action, args=(), close_button="ok", printNames=False):
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
            while True:
                if event.executed:
                    xDialog = self._xUITest.getTopFocusWindow()
                    try:
                        yield xDialog
                    except:
                        if not close_button:
                            if 'cancel' in xDialog.getChildren():
                                self.close_dialog_through_button(xDialog.getChild("cancel"), xDialog)
                        raise
                    finally:
                        if close_button:
                            self.close_dialog_through_button(xDialog.getChild(close_button), xDialog)
                        thread.join()
                    return
                time.sleep(DEFAULT_SLEEP)

# vim: set shiftwidth=4 softtabstop=4 expandtab:
