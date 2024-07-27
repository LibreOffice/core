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

from com.sun.star.awt import Size
from com.sun.star.text.ControlCharacter import PARAGRAPH_BREAK
from com.sun.star.text.TextContentAnchorType import AS_CHARACTER


def main():
    try:
        # Step 1: bootstrap UNO and get the remote component context. The
        #         context can be used to get the service manager.
        remote_context = officehelper.bootstrap()
        print("Connected to a running office ...")
        srv_mgr = remote_context.getServiceManager()
        desktop = srv_mgr.createInstanceWithContext("com.sun.star.frame.Desktop", remote_context)

        # Step 2: open an empty document. In this case it's a writer document.
        #         For this purpose an instance of com.sun.star.frame.Desktop is
        #         created. It's interface XDesktop provides the XComponentLoader,
        #         which is used to open the document via loadComponentFromURL
        print("Opening an empty Writer document")
        doc_url = "private:factory/swriter"
        doc = desktop.loadComponentFromURL(doc_url, "_blank", 0, tuple())
    except Exception:
        print("d", file=sys.stderr)
        traceback.print_exc()
        sys.exit(1)

    generate(doc)


def generate(doc):
    # Step 3: insert some text
    text = doc.getText()
    cursor = text.createTextCursor()

    text.insertString(cursor, "The first line in the newly created text document.\n", False)
    text.insertString(cursor, "Now we're in the second line\n", False)

    # Step 4: insert a text table
    insert_table(doc, text, cursor)

    # Step 5: insert a colored text
    try:
        cursor.setPropertyValue("CharColor", 255)
        cursor.setPropertyValue("CharShadowed", True)
    except Exception:
        print("Couldn't change the color", file=sys.stderr)
        traceback.print_exc()

    try:
        text.insertControlCharacter(cursor, PARAGRAPH_BREAK, False)
    except Exception as e:
        print(f"Couldn't insert break: {e}", file=sys.stderr)
        traceback.print_exc()

    print("Inserting colored Text")
    text.insertString(cursor, " This is a colored Text - blue with shadow\n", False)

    try:
        text.insertControlCharacter(cursor, PARAGRAPH_BREAK, False)
    except Exception as e:
        print(f"Couldn't insert break: {e}", file=sys.stderr)
        traceback.print_exc()

    # Step 6: insert a text frame
    insert_frame_with_text(doc, text, cursor)

    try:
        text.insertControlCharacter(cursor, PARAGRAPH_BREAK, False)
    except Exception as e:
        print(f"Couldn't insert break: {e}", file=sys.stderr)
        traceback.print_exc()

    try:
        cursor.setPropertyValue("CharColor", 65536)
        cursor.setPropertyValue("CharShadowed", False)
    except Exception as e:
        print(f"Couldn't change the color: {e}", file=sys.stderr)

    text.insertString(cursor, " That's all for now !!", False)

    print("done")


def insert_table(doc, text, cursor):
    print("Inserting a text table")
    try:
        text_table = doc.createInstance("com.sun.star.text.TextTable")
    except Exception as e:
        print(f"Couldn't create instance of TextTable: {e}", file=sys.stderr)
        traceback.print_exc()
        return

    # initialize the text table with 4 columns an 4 rows
    text_table.initialize(4, 4)

    try:
        text.insertTextContent(cursor, text_table, False)
    except Exception as e:
        print(f"Couldn't insert the table: {e}", file=sys.stderr)
        traceback.print_exc()
        return

    # Get the first row
    rows = text_table.getRows()
    first_row = rows[0]

    try:
        # Set properties of the text table
        text_table.setPropertyValue("BackTransparent", False)
        text_table.setPropertyValue("BackColor", 13421823)
        # Set properties of the first row
        first_row.setPropertyValue("BackTransparent", False)
        first_row.setPropertyValue("BackColor", 6710932)
    except Exception as e:
        print(f"Couldn't change the color: {e}", file=sys.stderr)
        traceback.print_exc()

    print("Write text in the table headers")
    insert_into_cell("A1", "FirstColumn", text_table)
    insert_into_cell("B1", "SecondColumn", text_table)
    insert_into_cell("C1", "ThirdColumn", text_table)
    insert_into_cell("D1", "SUM", text_table)

    print("Insert something in the text table")
    data = (
        ("A2", 22.5, False),
        ("B2", 5615.3, False),
        ("C2", -2315.7, False),
        ("D2", "sum <A2:C2>", True),
        ("A3", 21.5, False),
        ("B3", 615.3, False),
        ("C3", -315.7, False),
        ("D3", "sum <A3:C3>", True),
        ("A4", 121.5, False),
        ("B4", -615.3, False),
        ("C4", 415.7, False),
        ("D4", "sum <A4:C4>", True),
    )
    for cell_name, value, is_formula in data:
        cell = text_table.getCellByName(cell_name)
        if is_formula:
            cell.setFormula(value)
        else:
            cell.setValue(value)


def insert_frame_with_text(doc, text, cursor):
    try:
        text_frame = doc.createInstance("com.sun.star.text.TextFrame")
        frame_size = Size()
        frame_size.Height = 400
        frame_size.Width = 15000
        text_frame.setSize(frame_size)
    except Exception as e:
        print(f"Couldn't create instance: {e}", file=sys.stderr)
        traceback.print_exc()
        return

    # Change the AnchorType
    try:
        text_frame.setPropertyValue("AnchorType", AS_CHARACTER)
    except Exception as e:
        print(f"Couldn't change the color: {e}", file=sys.stderr)
        traceback.print_exc()

    print("Insert the text frame")

    try:
        text.insertTextContent(cursor, text_frame, False)
    except Exception as e:
        print(f"Couldn't insert the frame: {e}", file=sys.stderr)
        traceback.print_exc()

    frame_text = text_frame.getText()
    frame_cursor = frame_text.createTextCursor()
    s = "The first line in the newly created text frame."
    text_frame.insertString(frame_cursor, s, False)
    s = "\nWith this second line the height of the frame raises."
    text_frame.insertString(frame_cursor, s, False)


def insert_into_cell(cell_name: str, content: str, text_table):
    cell = text_table.getCellByName(cell_name)
    cursor = cell.createTextCursor()
    try:
        cursor.setPropertyValue("CharColor", 16777215)
    except Exception as e:
        print(f"Fail to set CharColor property: {e}", file=sys.stderr)
        traceback.print_exc()
    # inserting some Text
    cell.setString(content)


if __name__ == "__main__":
    main()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
