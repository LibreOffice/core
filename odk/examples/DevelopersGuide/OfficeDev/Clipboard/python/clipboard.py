# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import time
import traceback

import officehelper

from clipboard_owner import ClipboardOwner
from clipboard_listener import ClipboardListener
from text_transferable import TextTransferable

from com.sun.star.datatransfer import UnsupportedFlavorException

# This example demonstrates the usage of the clipboard service

def demonstrate_clipboard():
    try:
        # Create a new office context (if needed), and get a reference to it
        context = officehelper.bootstrap()
        print("Connected to a running office ...")

        # Get a reference to the multi component factory/service manager
        srv_mgr = context.getServiceManager()

        # Create a new blank document, and write user instructions to it
        desktop = srv_mgr.createInstanceWithContext("com.sun.star.frame.Desktop", context)
        doc = desktop.loadComponentFromURL("private:factory/swriter", "_blank", 0, tuple())
        doc.getText().setString("In the first step, paste the current content of the clipboard in the document!\nThe text \"Hello world!\" shall be insert at the current cursor position below.\n\nIn the second step, please select some words and put it into the clipboard! ...\n\nCurrent clipboard content = ")

        # Get the current controller, and ensure that the document
        # content is visible to the user by zooming in
        view_settings_supplier = doc.getCurrentController()
        view_settings_supplier.getViewSettings() \
            .setPropertyValue("ZoomType", 0)

        # Create an instance of systemclipboard that abstracts the
        # low level system-specific clipboard
        clipboard = srv_mgr.createInstance("com.sun.star.datatransfer.clipboard.SystemClipboard")

        # Create a listener for the clipboard
        print("Registering a clipboard listener...")
        clip_listener = ClipboardListener()
        clipboard.addClipboardListener(clip_listener)

        read_clipboard(clipboard)

        # Create an owner for the clipboard, and "copy" something to it
        print("Becoming a clipboard owner...")
        clip_owner = ClipboardOwner()
        clipboard.setContents(TextTransferable("Hello World!"), clip_owner)

        # Show a hint to the user running the example for what to do
        # as an ellipses style indicator
        i = 0
        while clip_owner.isClipboardOwner():
            if i != 2:
                if i == 1:
                    print("Change clipboard ownership by putting something into the clipboard!")
                    print("Still clipboard owner...", end='')
                else:
                    print("Still clipboard owner...", end='')
                i += 1
            else:
                print(".", end='')
            time.sleep(1)
        print()

        read_clipboard(clipboard)

        # End the clipboard listener
        print("Unregistering a clipboard listener...")
        clipboard.removeClipboardListener(clip_listener)

        # Close the temporary test document
        doc.close(False)

    except Exception as e:
        print(str(e))
        traceback.print_exc()


def read_clipboard(clipboard):
    # get a list of formats currently on the clipboard
    transferable = clipboard.getContents()
    data_flavors_list = transferable.getTransferDataFlavors()

    # print all available formats

    print("Reading the clipboard...")
    print("Available clipboard formats:", data_flavors_list)

    unicode_flavor = None

    for flavor in data_flavors_list:
        print("MimeType:", flavor.MimeType,
            "HumanPresentableName:", flavor.HumanPresentableName)

        # Select the flavor that supports utf-16
        # str.casefold() allows reliable case-insensitive comparision
        if flavor.MimeType.casefold() == TextTransferable.UNICODE_CONTENT_TYPE.casefold():
            unicode_flavor = flavor
    print()

    try:
        if unicode_flavor is not None:
            # Get the clipboard data as unicode
            data = transferable.getTransferData(unicode_flavor)

            print("Unicode text on the clipboard ...")
            print(f"Your selected text \"{data}\" is now in the clipboard.")

    except UnsupportedFlavorException as ex:
        print("Requested format is not available on the clipboard!")
        print(str(ex))
        traceback.print_exc()


if __name__ == "__main__":
    demonstrate_clipboard()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
