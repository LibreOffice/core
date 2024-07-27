# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import officehelper
import sys
import traceback

from com.sun.star.text.ControlCharacter import PARAGRAPH_BREAK


def get_desktop():
    desktop = None
    try:
        remote_context = officehelper.bootstrap()
        srv_mgr = remote_context.getServiceManager()
        if srv_mgr is None:
            print("Can't create a desktop. No connection, no remote office servicemanager available!")
        else:
            desktop = srv_mgr.createInstanceWithContext("com.sun.star.frame.Desktop", remote_context)
    except Exception:
        traceback.print_exc()
        sys.exit(1)
    return desktop


def main():
    desktop = get_desktop()
    if desktop is None:
        return

    print("Opening an empty Writer document")

    try:
        doc = desktop.loadComponentFromURL("private:factory/swriter", "_blank", 0, tuple())
    except Exception:
        traceback.print_exc()
        sys.exit(1)

    create_example_data(doc)

    british_words = ["colour", "neighbour", "centre", "behaviour", "metre", "through"]
    us_words = ["color", "neighbor", "center", "behavior", "meter", "thru"]

    try:
        replace_descriptor = doc.createReplaceDescriptor()
        print("Change all occurrences of ...")
        for british_word, us_word in zip(british_words, us_words):
            replace_descriptor.setSearchString(british_word)
            replace_descriptor.setReplaceString(us_word)
            # Replace all words
            replaced_cnt = doc.replaceAll(replace_descriptor)
            if replaced_cnt > 0:
                print("Replaced", british_word, "with", us_word)
    except Exception:
        traceback.print_exc()

    print("Done")


def create_example_data(doc):
    try:
        text = doc.getText()
        cursor = text.createTextCursor()
        text.insertString(cursor, "He nervously looked all around. Suddenly he saw his ", False)

        text.insertString(cursor, "neighbour ", True)
        cursor.setPropertyValue("CharColor", 255)  # Set the word blue

        cursor.gotoEnd(False)  # Go to last character
        cursor.setPropertyValue("CharColor", 0)
        content = (
            "in the alley. Like lightning he darted off to the left and disappeared between the "
            "two warehouses almost falling over the trash can lying in the "
        )
        text.insertString(cursor, content, False)

        text.insertString(cursor, "centre ", True)
        cursor.setPropertyValue("CharColor", 255)  # Set the word blue

        cursor.gotoEnd(False)  # Go to last character
        cursor.setPropertyValue("CharColor", 0)
        text.insertString(cursor, "of the sidewalk.", False)

        text.insertControlCharacter(cursor, PARAGRAPH_BREAK, False)
        content = (
            "He tried to nervously tap his way along in the inky darkness and suddenly stiffened: "
            "it was a dead-end, he would have to go back the way he had come."
        )
        text.insertString(cursor, content, False)
        cursor.gotoStart(False)
    except Exception:
        traceback.print_exc()


if __name__ == "__main__":
    main()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
