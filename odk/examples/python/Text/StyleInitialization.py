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

    try:
        doc_url = "private:factory/swriter"
        doc = desktop.loadComponentFromURL(doc_url, "_blank", 0, tuple())

        text = doc.getText()
        cursor = text.createTextCursor()

        try:
            cursor.setPropertyValue("CharFontName", "Arial")
        except Exception:
            pass
        text.insertString(cursor, "Headline", False)

        try:
            cursor.setPropertyValue("CharFontName", "Liberation Sans")
        except Exception:
            pass
        text.insertControlCharacter(cursor, PARAGRAPH_BREAK, False)
        text.insertString(cursor, "A very short paragraph for illustration only", False)

        # The text range not the cursor contains the 'ParaStyleName' property
        text_range = text.getEnd()
        # To run the sample with StarOffice 5.2 you'll have to change
        # 'ParaStyleName' to 'ParaStyle' in the next line
        print("Current Parastyle:", text_range.getPropertyValue("ParaStyleName"))

        # There are two way to travel through the paragraphs, with a paragraph
        # cursor, or an enumeration. You find both ways in this example

        # The first way, with the paragraph cursor
        # Object text_range supports interface com.sun.star.text.XParagraphCursor
        text_range.gotoStart(False)
        text_range.gotoEndOfParagraph(True)

        # The second way, with the paragraph enumeration
        paragraph_enumeration = text.createEnumeration()
        while paragraph_enumeration.hasMoreElements():
            paragraph = paragraph_enumeration.nextElement()
            # Create a cursor from this paragraph
            paragraph_cursor = paragraph.getAnchor().getText().createTextCursor()

            # Goto the start and end of the paragraph
            paragraph_cursor.gotoStart(False)
            paragraph_cursor.gotoEnd(True)

            portion_enumeration = paragraph.createEnumeration()
            while portion_enumeration.hasMoreElements():
                word = portion_enumeration.nextElement()
                print("Content of the paragraph:", word.getString())

        # Find a paragraph style by a specific font name and apply the found
        # style to paragraph.
        style_families = doc.getStyleFamilies()
        styles = style_families["ParagraphStyles"]
        for style_name in styles.getElementNames():
            style = styles[style_name]
            font_name = style.getPropertyValue("CharFontName").lower()
            if font_name == "liberation mono":
                text_range.setPropertyValue("ParaStyleName", style_name)
                print("Apply the paragraph style:", style_name)
                break
    except Exception:
        traceback.print_exc()

    print("Done")


if __name__ == "__main__":
    main()


# vim: set shiftwidth=4 softtabstop=4 expandtab:
