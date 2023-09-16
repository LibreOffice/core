'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Function new_doc_component(doc_type As String)
    load_url = "private:factory/" & doc_type
    desktop = createUnoService("com.sun.star.frame.Desktop")
    Set new_doc_component = desktop.loadComponentFromURL(load_url, "_blank", 0, Array())
End Function

Sub use_documents
    use_writer()
    use_calc()
    use_draw()
End Sub

Sub use_writer
    Set doc = new_doc_component("swriter")
    Set xtext = doc.Text
    manipulateText(xtext)

    ' insert TextTable and get cell text, then manipulate text in cell
    Set table = doc.createInstance("com.sun.star.text.TextTable")
    xtext.insertTextContent(xtext.End, table, False)

    xcell = table.getCellByPosition(0, 1)
    manipulateText(xcell.getText())
    manipulateTable(table)

    ' insert RectangleShape and get shape text, then manipulate text
    Set writer_shape = doc.createInstance("com.sun.star.drawing.RectangleShape")

    Dim Point As New com.sun.star.awt.Point
    Dim Size As New com.sun.star.awt.Size
    Size.Width= 10000
    Size.Height= 10000
    writer_shape.setSize(Size)
    xtext.insertTextContent(xtext.End, writer_shape, False)
    ' wrap text inside shape
    writer_shape.TextContourFrame = True

    manipulateText(writer_shape)
    manipulateShape(writer_shape)

    bookmark = doc.createInstance("com.sun.star.text.Bookmark")
    bookmark.Name = "MyUniqueBookmarkName"
    ' insert the bookmark at the end of the document
    xtext.insertTextContent(xtext.End, bookmark, False)

    ' Query the added bookmark and set a string
    found_bookmark = doc.Bookmarks.getByName("MyUniqueBookmarkName")
    found_bookmark.Anchor.String = _
        "The throat mike, glued to her neck, " +_
        "looked as much as possible like an analgesic dermadisk." _

    Set text_table = doc.TextTables
    For i = 0 To text_table.getCount() - 1
         text_table.getByIndex(i).BackColor = &HC8FFB9
    Next
End Sub

Sub use_calc
    doc = new_doc_component("scalc")
    sheet = doc.Sheets(0)

    ' get cell A2 in first sheet
    cell = sheet.getCellByPosition(1, 0)
    cell.IsTextWrapped = True

    manipulateText(cell.getText())
    manipulateTable(sheet)

    ' create and insert RectangleShape and get shape text,
    ' then manipulate text
    shape = doc.createInstance("com.sun.star.drawing.RectangleShape")

    Dim Point As New com.sun.star.awt.Point
    Dim Size As New com.sun.star.awt.Size

    shape = doc.createInstance("com.sun.star.drawing.RectangleShape")
    Point.X = 7000
    Point.Y = 3000
    Size.Width= 10000
    Size.Height= 10000
    shape.setSize(Size)
    shape.setPosition(Point)

    shape.TextContourFrame = True
    sheet.DrawPage.add(shape)

    manipulateText(shape)
    manipulateShape(shape)
End Sub

Sub use_draw
    doc = new_doc_component("sdraw")

    Dim Point As New com.sun.star.awt.Point
    Dim Size As New com.sun.star.awt.Size

    draw_shape = doc.createInstance("com.sun.star.drawing.RectangleShape")
    Point.X = 5000
    Point.Y = 5000
    Size.Width= 10000
    Size.Height= 10000
    draw_shape.setSize(Size)
    draw_shape.setPosition(Point)
    doc.DrawPages(0).add(draw_shape)

    ' wrap text inside shape
    draw_shape.TextContourFrame = True

    manipulateText(draw_shape)
    manipulateShape(draw_shape)
End Sub

Sub manipulateText(xtext As Object)
    ' Insert text content

    'param xtext: object that implements com.sun.star.text.XText interface.

    ' simply set whole text as one string
    xtext.String = "He lay flat on the brown, pine-needled floor of the forest, " +_
        "his chin on his folded arms, and high overhead the wind blew " +_
        "in the tops of the pine trees."

    ' create text cursor for selecting and formatting
    text_cursor = xtext.createTextCursor()
    ' use cursor to select "He lay" and apply bold italic
    text_cursor.gotoStart(False)
    text_cursor.goRight(6, True)
    ' from CharacterProperties
    text_cursor.CharPosture = com.sun.star.awt.FontSlant.ITALIC
    text_cursor.CharWeight = 150

    ' add more text at the end of the text using insertString
    text_cursor.gotoEnd(False)
    content = _
        " The mountainside sloped gently where he lay; " +_
        "but below it was steep and he could see the dark of the oiled " +_
        "road winding through the pass. There was a stream alongside the " +_
        "road and far down the pass he saw a mill beside the stream and " +_
        "the falling water of the dam, white in the summer sunlight."

    xtext.insertString(text_cursor, content, False)
    ' after insertString the cursor is behind the inserted text,
    ' insert more text
    content = CHR$(10) & "  ""Is that the mill?"" he asked."
    xtext.insertString(text_cursor, content, False)
End Sub

Sub manipulateTable(xcellrange As Object)
    'Format a table area

    ':param xcellrange: object that implements com.sun.star.table.XCellRange interface.

    ' enter column titles and a cell value
    xcellrange.getCellByPosition(0, 0).SetString("Quotation")
    xcellrange.getCellByPosition(0, 1).SetString("Year")
    xcellrange.getCellByPosition(1, 1).SetValue(1940)

    ' format table headers and table borders
    ' we need to distinguish text and sheet tables:
    ' property name for cell colors is different in text and sheet cells
    ' we want to apply TableBorder to whole text table, but only to sheet
    ' cells with content

    background_color = &H99CCFF

    ' create description for blue line, width 10
    Dim border_line As New com.sun.star.table.BorderLine
    border_line.Color = &H000099
    border_line.OuterLineWidth = 10
    ' apply line description to all border lines and make them valid
    Dim border As New com.sun.star.table.TableBorder
    border.VerticalLine = border_line
    border.HorizontalLine = border_line
    border.LeftLine = border_line
    border.RightLine = border_line
    border.TopLine = border_line
    border.BottomLine = border_line
    border.IsVerticalLineValid = True
    border.IsHorizontalLineValid = True
    border.IsLeftLineValid = True
    border.IsRightLineValid = True
    border.IsTopLineValid = True
    border.IsBottomLineValid = True


    If xcellrange.supportsService("com.sun.star.sheet.Spreadsheet") Then
        selected_cells = xcellrange.getCellRangeByName("A1:B2")
        selected_cells.CellBackColor = background_color
        selected_cells.TableBorder = border
'        Print selected_cells.TableBorder.TopLine.Color
    ElseIf xcellrange.supportsService("com.sun.star.text.TextTable") Then
        selected_cells = xcellrange.getCellRangeByName("A1:B1")
        selected_cells.BackColor = background_color
        xcellrange.TableBorder = border
'        Print xcellrange.TableBorder.TopLine.Color
    End If
End Sub

Sub manipulateShape(xshape As Object)
    'Format a shape
    'param xshape: object that implements com.sun.star.drawing.XShape interface.

    xshape.FillColor = &H99CCFF
    xshape.LineColor = &H000099
    xshape.RotateAngle = 3000

    xshape.TextLeftDistance = 0
    xshape.TextRightDistance = 0
    xshape.TextUpperDistance = 0
    xshape.TextLowerDistance = 0
End Sub

Sub Main
    use_documents()
End Sub