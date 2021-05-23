# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from contextlib import contextmanager

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

# vim: set shiftwidth=4 softtabstop=4 expandtab:
