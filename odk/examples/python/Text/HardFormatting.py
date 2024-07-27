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

from com.sun.star.awt.FontWeight import BOLD
from com.sun.star.beans.PropertyState import AMBIGUOUS_VALUE
from com.sun.star.beans.PropertyState import DEFAULT_VALUE
from com.sun.star.beans.PropertyState import DIRECT_VALUE
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

    try:
        doc = desktop.loadComponentFromURL("private:factory/swriter", "_blank", 0, tuple())

        text = doc.getText()
        cursor = text.createTextCursor()

        text.insertString(cursor, "Headline", False)
        text.insertControlCharacter(cursor, PARAGRAPH_BREAK, False)
        text.insertString(cursor, "A very short paragraph for illustration only", False)

        # Start 'Hard formatting'
        # the text range not the cursor contains the 'parastyle' property
        text_range = text.getEnd()
        text_range.gotoStart(False)
        text_range.gotoEndOfParagraph(True)

        # Later, we will go through words in this text range
        text_range = text_range.getText().getStart()
        # Display the current text attributes
        print("Parastyle:", text_range.getPropertyValue("ParaStyleName"))
        print("Fontname: ", text_range.getPropertyValue("CharFontName"))
        print("Weight:   ", text_range.getPropertyValue("CharWeight"))

        # Move around
        text_range.gotoNextWord(False)
        text_range.gotoNextWord(False)
        text_range.gotoEndOfWord(True)
        # And set text attributes
        text_range.setPropertyValue("CharWeight", BOLD)
        text_range.setPropertyValue("CharColor", 255)
        # Then, display the text attributes
        print("Parastyle:", text_range.getPropertyValue("ParaStyleName"))
        print("Fontname: ", text_range.getPropertyValue("CharFontName"))
        print("Weight:   ", text_range.getPropertyValue("CharWeight"))

        # the PropertyState contains information where the attribute is set,
        # is a text part hard formatted or not.
        check_property_state(text_range, text_range.getPropertyState("CharWeight"))

        print("Increase the selection with three characters")
        text_range.goRight(3, True)
        check_property_state(text_range, text_range.getPropertyState("CharWeight"))

        print("Set the default value on the selection")
        text_range.setPropertyToDefault("CharWeight")
        # Then, check again
        check_property_state(text_range, text_range.getPropertyState("CharWeight"))
    except Exception:
        traceback.print_exc()
        sys.exit(1)

    print("Done")


def check_property_state(text_range, prop_state):
    if prop_state == DIRECT_VALUE:
        print("-> The selection", f"'{text_range.getString()}'", "completely hard formatted")
    elif prop_state == DEFAULT_VALUE:
        print("-> The selection", f"'{text_range.getString()}'", "isn't hard formatted")
    elif prop_state == AMBIGUOUS_VALUE:
        print("-> The selection", f"'{text_range.getString()}'", "isn't completely hard formatted")
    else:
        print("No PropertyState found")


if __name__ == "__main__":
    main()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
