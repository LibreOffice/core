# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import time
from contextlib import contextmanager
import threading
from uitest.config import DEFAULT_SLEEP
from uitest.config import MAX_WAIT
from uitest.test import DialogNotExecutedException

from libreoffice.uno.eventlistener import EventListener

# Calls UITest.close_doc at exit
@contextmanager
def load_file(testCase, url):
    component = testCase.ui_test.load_file(url)
    try:
        yield component
    finally:
        testCase.ui_test.close_doc()

# Calls UITest.close_doc at exit
@contextmanager
def create_doc_in_start_center(testCase, app):
    testCase.ui_test.create_doc_in_start_center(app)
    component = testCase.ui_test.get_component()
    try:
        yield component
    finally:
        testCase.ui_test.close_doc()

# Calls UITest.close_dialog_through_button at exit
@contextmanager
def execute_dialog_through_action(testCase, ui_object, action, parameters = None, event_name = "DialogExecute", close_button = "ok"):
    testCase.ui_test.execute_dialog_through_action(ui_object, action, parameters, event_name)
    xDialog = testCase.xUITest.getTopFocusWindow()
    try:
        yield xDialog
    finally:
        testCase.ui_test.close_dialog_through_button(xDialog.getChild(close_button))

# Calls UITest.close_dialog_through_button at exit
@contextmanager
def execute_dialog_through_command(testCase, command, printNames=False, close_button = "ok"):
    testCase.ui_test.execute_dialog_through_command(command, printNames)
    xDialog = testCase.xUITest.getTopFocusWindow()
    try:
        yield xDialog
    finally:
        testCase.ui_test.close_dialog_through_button(xDialog.getChild(close_button))

# Executes an action which blocks while a dialog is shown.
#
# Args:
#     action(callable): Will be called to show a dialog, and is expected
#         to block while the dialog is shown.
#     args(tuple, optional): The arguments to be passed to `action`
#     close_button(str, optional): The name of a button on the dialog
#         which will be clicked to close the dialog.
#     printNames(boolean, optional): print all received event names
@contextmanager
def execute_blocking_action(testCase, action, args=(), close_button="ok", printNames=False):
    thread = threading.Thread(target=action, args=args)
    with EventListener(testCase.ui_test._xContext, ["DialogExecute", "ModelessDialogExecute", "ModelessDialogVisible"], printNames=printNames) as event:
        thread.start()
        time_ = 0
        # we are not necessarily opening a dialog, so wait much longer
        while time_ < 10 * MAX_WAIT:
            if event.executed:
                xDialog = testCase.xUITest.getTopFocusWindow()
                try:
                    yield xDialog
                finally:
                    testCase.ui_test.close_dialog_through_button(xDialog.getChild(close_button))
                    thread.join()
                    return

            time_ += DEFAULT_SLEEP
            time.sleep(DEFAULT_SLEEP)
    raise DialogNotExecutedException("did not execute a dialog for a blocking action")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
