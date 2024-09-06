' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.

Imports System

Imports com.sun.star.awt
Imports com.sun.star.beans
Imports com.sun.star.bridge
Imports com.sun.star.container
Imports com.sun.star.drawing
Imports com.sun.star.frame
Imports com.sun.star.lang
Imports com.sun.star.text
Imports com.sun.star.table
Imports com.sun.star.uno

Module WriterDemo
    Sub Main()
        ' Connect to a running office
        Dim context As XComponentContext = NativeBootstrap.bootstrap()
        ' Create a service manager of the remote office
        Dim factory As XMultiComponentFactory = context.getServiceManager()

        ' Create the Desktop
        Dim desktop_ As XDesktop = Desktop.create(context)

        ' Open a new empty writer document
        Dim componentLoader As XComponentLoader = desktop_.query(Of XComponentLoader)()
        Dim component As XComponent = componentLoader.loadComponentFromURL(
            "private:factory/swriter", "_blank", 0, Array.Empty(Of PropertyValue)())

        Dim textDocument As XTextDocument = component.query(Of XTextDocument)()

        ' Create a text object
        Dim text As XText = textDocument.getText()
        Dim simpleText As XSimpleText = text.query(Of XSimpleText)()

        ' Create a cursor object
        Dim cursor As XTextCursor = simpleText.createTextCursor()

        ' Inserting some Text
        text.insertString(cursor, "The first line in the newly created text document." & vbLf, False)

        ' Create instance of a text table with 4 columns and 4 rows
        Dim textTableI As IQueryInterface = textDocument.query(Of XMultiServiceFactory)().createInstance("com.sun.star.text.TextTable")
        Dim textTable As XTextTable = textTableI.query(Of XTextTable)()
        textTable.initialize(4, 4)
        text.insertTextContent(cursor, textTable, False)

        ' Set the table background color
        Dim tablePropertySet As XPropertySet = textTableI.query(Of XPropertySet)()
        tablePropertySet.setPropertyValue("BackTransparent", New Any(False))
        tablePropertySet.setPropertyValue("BackColor", New Any(&HCCCCFF))

        ' Get first row
        Dim tableRows As XTableRows = textTable.getRows()
        Dim rowAny As Any = tableRows.query(Of XIndexAccess)().getByIndex(0)

        ' Set a different background color for the first row
        Dim firstRowPropertySet As XPropertySet = rowAny.cast(Of XPropertySet)()
        firstRowPropertySet.setPropertyValue("BackTransparent", New Any(False))
        firstRowPropertySet.setPropertyValue("BackColor", New Any(&H6666AA))

        ' Fill the first table row
        InsertIntoCell("A1", "FirstColumn", textTable)
        InsertIntoCell("B1", "SecondColumn", textTable)
        InsertIntoCell("C1", "ThirdColumn", textTable)
        InsertIntoCell("D1", "SUM", textTable)

        ' Fill the remaining rows
        textTable.getCellByName("A2").setValue(22.5)
        textTable.getCellByName("B2").setValue(5615.3)
        textTable.getCellByName("C2").setValue(-2315.7)
        textTable.getCellByName("D2").setFormula("sum <A2:C2>")

        textTable.getCellByName("A3").setValue(21.5)
        textTable.getCellByName("B3").setValue(615.3)
        textTable.getCellByName("C3").setValue(-315.7)
        textTable.getCellByName("D3").setFormula("sum <A3:C3>")

        textTable.getCellByName("A4").setValue(121.5)
        textTable.getCellByName("B4").setValue(-615.3)
        textTable.getCellByName("C4").setValue(415.7)
        textTable.getCellByName("D4").setFormula("sum <A4:C4>")

        ' Change the CharColor and add a Shadow
        Dim cursorPropertySet As XPropertySet = cursor.query(Of XPropertySet)()
        cursorPropertySet.setPropertyValue("CharColor", New Any(255))
        cursorPropertySet.setPropertyValue("CharShadowed", New Any(True))

        ' Create a paragraph break
        simpleText.insertControlCharacter(cursor, ControlCharacter.PARAGRAPH_BREAK, False)

        ' Inserting colored Text.
        simpleText.insertString(cursor, " This is a colored Text - blue with shadow" & vbLf, False)

        ' Create a paragraph break
        simpleText.insertControlCharacter(cursor, ControlCharacter.PARAGRAPH_BREAK, False)

        ' Create a TextFrame.
        Dim textFrameI As IQueryInterface = textDocument.query(Of XMultiServiceFactory)().createInstance("com.sun.star.text.TextFrame")
        Dim textFrame As XTextFrame = textFrameI.query(Of XTextFrame)()

        ' Set the size of the frame
        Dim size As Size = New Size(15000, 400)
        textFrame.query(Of XShape)().setSize(size)

        ' Set anchortype
        Dim framePropertySet As XPropertySet = textFrame.query(Of XPropertySet)()
        framePropertySet.setPropertyValue("AnchorType", New Any(TextContentAnchorType.AS_CHARACTER))

        ' Insert the frame
        text.insertTextContent(cursor, textFrame, False)

        ' Get the text object of the frame
        Dim frameText As XText = textFrame.getText()
        Dim frameSimpleText As XSimpleText = frameText.query(Of XSimpleText)()

        ' Create a cursor object
        Dim frameCursor As XTextCursor = frameSimpleText.createTextCursor()

        ' Inserting some Text
        frameSimpleText.insertString(frameCursor, "The first line in the newly created text frame.", False)
        frameSimpleText.insertString(frameCursor, vbLf & "With this second line the height of the frame raises.", False)

        ' Create a paragraph break
        simpleText.insertControlCharacter(frameCursor, ControlCharacter.PARAGRAPH_BREAK, False)

        ' Change the CharColor and add a Shadow
        cursorPropertySet.setPropertyValue("CharColor", New Any(65536))
        cursorPropertySet.setPropertyValue("CharShadowed", New Any(False))

        ' Insert another string
        text.insertString(cursor, vbLf & " That's all for now !!", False)
    End Sub

    Private Sub InsertIntoCell(cellName As String, text As String, textTable As XTextTable)
        Dim cell As XCell = textTable.getCellByName(cellName)
        Dim simpleText As XSimpleText = cell.query(Of XSimpleText)()
        simpleText.setString(text)
    End Sub
End Module