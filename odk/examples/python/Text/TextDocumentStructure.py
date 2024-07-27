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


def main():
    try:
        remote_context = officehelper.bootstrap()
        srv_mgr = remote_context.getServiceManager()
        desktop = srv_mgr.createInstanceWithContext("com.sun.star.frame.Desktop", remote_context)

        doc_url = "private:factory/swriter"
        doc = desktop.loadComponentFromURL(doc_url, "_blank", 0, tuple())
        text = doc.getText()
        create_example_data(text)
        display_structure(text)
    except Exception:
        traceback.print_exc()
        sys.exit(1)

    print("done")


def create_example_data(text):
    try:
        text.setString("This is an example sentence")
        cursor = text.getStart()
        cursor.gotoNextWord(False)
        cursor.gotoNextWord(False)
        cursor.gotoEndOfWord(True)
        cursor.setPropertyValue("CharWeight", BOLD)
        print("create example data")
    except Exception:
        traceback.print_exc()


def display_structure(text):
    print("Document structure:")
    # Create an enumeration of all paragraphs
    paragraph_enum = text.createEnumeration()
    # Loop through all paragraphs of the document
    for element in paragraph_enum:
        if not element.supportsService("com.sun.star.text.Paragraph"):
            print("The text portion isn't a text paragraph")
            continue

        print("This is a Paragraph")
        for portion in element.createEnumeration():
            print("Text from the portion:", f"{portion.getString()}")
            print("Name of the font:", portion.getPropertyValue("CharFontName"))
            char_weight = portion.getPropertyState("CharWeight")
            if char_weight == AMBIGUOUS_VALUE:
                print(" -  The text range contains more than one different attributes")
            elif char_weight == DIRECT_VALUE:
                print(" - The text range contains hard formats")
            elif char_weight == DEFAULT_VALUE:
                print(" - The text range doesn't contains hard formats")
            else:
                pass  # Do nothing


if __name__ == "__main__":
    main()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
