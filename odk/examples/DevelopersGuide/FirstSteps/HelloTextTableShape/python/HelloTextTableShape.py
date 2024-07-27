# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

import sys
import traceback

import officehelper

from com.sun.star.lang import DisposedException
from com.sun.star.awt.FontSlant import ITALIC
from com.sun.star.awt.FontWeight import BOLD
from com.sun.star.table import BorderLine
from com.sun.star.table import TableBorder
from com.sun.star.awt import Size
from com.sun.star.awt import Point


class HelloTextTableShape:

    def __init__(self):
        self.remote_context = None
        self.remote_service_manager = None

    def use_documents(self) -> None:
        self.use_writer()
        self.use_calc()
        self.use_draw()

    def get_remote_service_manager(self) -> None:
        try:
            self.remote_context = officehelper.bootstrap()
            print("Connected to a running office ...")
            return self.remote_context.ServiceManager
        except Exception:
            traceback.print_exc()
            sys.exit(1)

    def new_doc_component(self, doc_type: str) -> None:
        load_url = "private:factory/" + doc_type
        self.remote_service_manager = self.get_remote_service_manager()
        desktop = self.remote_service_manager.createInstanceWithContext(
            "com.sun.star.frame.Desktop", self.remote_context
        )
        return desktop.loadComponentFromURL(load_url, "_blank", 0, tuple([]))

    def use_writer(self) -> None:
        try:
            doc = self.new_doc_component("swriter")
            xtext = doc.Text
            self.manipulateText(xtext)

            # insert TextTable and get cell text, then manipulate text in cell
            table = doc.createInstance("com.sun.star.text.TextTable")
            xtext.insertTextContent(xtext.End, table, False)

            xcell = table[1, 0]
            self.manipulateText(xcell)
            self.manipulateTable(table)

            # insert RectangleShape and get shape text, then manipulate text
            writer_shape = doc.createInstance("com.sun.star.drawing.RectangleShape")
            writer_shape.setSize(Size(10000, 10000))
            xtext.insertTextContent(xtext.End, writer_shape, False)
            # wrap text inside shape
            writer_shape.TextContourFrame = True

            self.manipulateText(writer_shape)
            self.manipulateShape(writer_shape)

            bookmark = doc.createInstance("com.sun.star.text.Bookmark")
            bookmark.Name = "MyUniqueBookmarkName"
            # insert the bookmark at the end of the document
            xtext.insertTextContent(xtext.End, bookmark, False)

            # Query the added bookmark and set a string
            found_bookmark = doc.Bookmarks.getByName("MyUniqueBookmarkName")
            found_bookmark.Anchor.String = (
                "The throat mike, glued to her neck, "
                "looked as much as possible like an analgesic dermadisk."
            )

            for text_table in doc.TextTables:
                text_table.BackColor = 0xC8FFB9
        except DisposedException:
            self.remote_context = None
            raise

    def use_calc(self) -> None:
        try:
            doc = self.new_doc_component("scalc")
            sheet = doc.Sheets[0]

            # get cell A2 in first sheet
            cell = sheet[1, 0]
            cell.IsTextWrapped = True

            self.manipulateText(cell)
            self.manipulateTable(sheet)

            # create and insert RectangleShape and get shape text,
            # then manipulate text
            shape = doc.createInstance("com.sun.star.drawing.RectangleShape")
            shape.Size = Size(10000, 10000)
            shape.Position = Point(7000, 3000)
            shape.TextContourFrame = True
            sheet.DrawPage.add(shape)

            self.manipulateText(shape)
            self.manipulateShape(shape)
        except DisposedException:
            self.remote_context = None
            raise

    def use_draw(self) -> None:
        try:
            doc = self.new_doc_component("sdraw")

            draw_shape = doc.createInstance("com.sun.star.drawing.RectangleShape")
            draw_shape.setSize(Size(10000, 20000))
            draw_shape.setPosition(Point(5000, 5000))
            doc.DrawPages[0].add(draw_shape)

            # wrap text inside shape
            draw_shape.TextContourFrame = True

            self.manipulateText(draw_shape)
            self.manipulateShape(draw_shape)
        except DisposedException:
            self.remote_context = None
            raise

    def manipulateText(self, xtext) -> None:
        """Insert text content

        :param xtext: object that implements com.sun.star.text.XText interface.
        """
        # simply set whole text as one string
        xtext.String = (
            "He lay flat on the brown, pine-needled floor of the forest, "
            "his chin on his folded arms, and high overhead the wind blew "
            "in the tops of the pine trees."
        )

        # create text cursor for selecting and formatting
        text_cursor = xtext.createTextCursor()
        # use cursor to select "He lay" and apply bold italic
        text_cursor.gotoStart(False)
        text_cursor.goRight(6, True)
        # from CharacterProperties
        text_cursor.CharPosture = ITALIC
        text_cursor.CharWeight = BOLD

        # add more text at the end of the text using insertString
        text_cursor.gotoEnd(False)
        content = (
            " The mountainside sloped gently where he lay; "
            "but below it was steep and he could see the dark of the oiled "
            "road winding through the pass. There was a stream alongside the "
            "road and far down the pass he saw a mill beside the stream and "
            "the falling water of the dam, white in the summer sunlight."
        )
        xtext.insertString(text_cursor, content, False)
        # after insertString the cursor is behind the inserted text,
        # insert more text
        content = "\n  \"Is that the mill?\" he asked."
        xtext.insertString(text_cursor, content, False)

    def manipulateTable(self, xcellrange) -> None:
        """Format a table area

        :param xcellrange: object that implements com.sun.star.table.XCellRange interface.
        """
        # enter column titles and a cell value
        xcellrange[0, 0].String = "Quotation"
        xcellrange[0, 1].String = "Year"
        xcellrange[1, 1].Value = 1940

        # format table headers and table borders
        # we need to distinguish text and sheet tables:
        # property name for cell colors is different in text and sheet cells
        # we want to apply TableBorder to whole text table, but only to sheet
        # cells with content

        background_color = 0x99CCFF

        # create description for blue line, width 10
        line = BorderLine()
        line.Color = 0x000099
        line.OuterLineWidth = 10
        # apply line description to all border lines and make them valid
        border = TableBorder()
        border.VerticalLine = border.HorizontalLine = line
        border.LeftLine = border.RightLine = line
        border.TopLine = border.BottomLine = line
        border.IsVerticalLineValid = border.IsHorizontalLineValid = True
        border.IsLeftLineValid = border.IsRightLineValid = True
        border.IsTopLineValid = border.IsBottomLineValid = True

        if xcellrange.supportsService("com.sun.star.sheet.Spreadsheet"):
            selected_cells = xcellrange["A1:B2"]
            selected_cells.CellBackColor = background_color
            selected_cells.TableBorder = border
            print(selected_cells.TableBorder.TopLine.Color)
        elif xcellrange.supportsService("com.sun.star.text.TextTable"):
            selected_cells = xcellrange["A1:B1"]
            selected_cells.BackColor = background_color
            xcellrange.TableBorder = border
            print(xcellrange.TableBorder.TopLine.Color)

    def manipulateShape(self, xshape) -> None:
        """Format a shape

        :param xshape: object that implements com.sun.star.drawing.XShape interface.
        """
        xshape.FillColor = 0x99CCFF
        xshape.LineColor = 0x000099
        xshape.RotateAngle = 3000

        xshape.TextLeftDistance = 0
        xshape.TextRightDistance = 0
        xshape.TextUpperDistance = 0
        xshape.TextLowerDistance = 0


def main() -> None:
    try:
        HelloTextTableShape().use_documents()
    except Exception as e:
        print(str(e))
        traceback.print_exc()


if __name__ == "__main__":
    main()

# vim: set shiftwidth=4 softtabstop=4 expandtab:
