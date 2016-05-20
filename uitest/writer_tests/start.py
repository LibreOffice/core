# -*- Mode: python; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

from uitest_helper import UITest, get_state_as_dict

from helper import mkPropertyValues

import time

try:
    import pyuno
    import uno
    import unohelper
except ImportError:
    print("pyuno not found: try to set PYTHONPATH and URE_BOOTSTRAP variables")
    print("PYTHONPATH=/installation/opt/program")
    print("URE_BOOTSTRAP=file:///installation/opt/program/fundamentalrc")
    raise

def start_writer(xContext):
    xUITest = xContext.ServiceManager.createInstanceWithContext(
            "org.libreoffice.uitest.UITest", xContext)

    ui_test = UITest(xUITest, xContext)

    ui_test.create_doc_in_start_center("writer")

    xWriterDoc = xUITest.getTopFocusWindow()
    print(xWriterDoc.getChildren())

    xWriterEdit = xWriterDoc.getChild("writer_edit")
    print(xWriterEdit.getState())

    xWriterEdit.executeAction("SET", mkPropertyValues({"ZOOM": "200"}))

    time.sleep(2)

    ui_test.close_doc()

def type_text(xContext):
    xUITest = xContext.ServiceManager.createInstanceWithContext(
            "org.libreoffice.uitest.UITest", xContext)

    ui_test = UITest(xUITest, xContext)

    ui_test.create_doc_in_start_center("writer")

    xWriterDoc = xUITest.getTopFocusWindow()
    xWriterEdit = xWriterDoc.getChild("writer_edit")

    xWriterEdit.executeAction("TYPE", mkPropertyValues({"TEXT": "This is my first writer text written through the UI testing"}))

    time.sleep(2)

    ui_test.close_doc()

def goto_first_page(xContext):
    xUITest = xContext.ServiceManager.createInstanceWithContext(
            "org.libreoffice.uitest.UITest", xContext)

    ui_test = UITest(xUITest, xContext)

    ui_test.create_doc_in_start_center("writer")

    xWriterDoc = xUITest.getTopFocusWindow()
    xWriterEdit = xWriterDoc.getChild("writer_edit")

    state = get_state_as_dict(xWriterEdit)
    print(state)
    while state["CurrentPage"] is "1":
        xWriterEdit.executeAction("TYPE", mkPropertyValues({"KEYCODE": "RETURN"}))
        state = get_state_as_dict(xWriterEdit)

    xWriterEdit.executeAction("GOTO", mkPropertyValues({"PAGE": "1"}))
    print(state)
    time.sleep(2)

    ui_test.close_doc()


def select_text(xContext):
    xUITest = xContext.ServiceManager.createInstanceWithContext(
            "org.libreoffice.uitest.UITest", xContext)

    ui_test = UITest(xUITest, xContext)

    ui_test.create_doc_in_start_center("writer")

    xWriterDoc = xUITest.getTopFocusWindow()
    xWriterEdit = xWriterDoc.getChild("writer_edit")

    xWriterEdit.executeAction("TYPE", mkPropertyValues({"TEXT": "This is my first writer text written through the UI testing"}))

    time.sleep(2)
    print(get_state_as_dict(xWriterEdit))

    xWriterEdit.executeAction("SELECT", mkPropertyValues({"START_POS": "0", "END_POS": "4"}))

    print(get_state_as_dict(xWriterEdit))

    time.sleep(2)

    ui_test.close_doc()

# vim:set shiftwidth=4 softtabstop=4 expandtab: */
