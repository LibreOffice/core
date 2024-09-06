// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

open System
open com.sun.star.awt
open com.sun.star.beans
open com.sun.star.bridge
open com.sun.star.container
open com.sun.star.drawing
open com.sun.star.frame
open com.sun.star.lang
open com.sun.star.text
open com.sun.star.table
open com.sun.star.uno

let InsertIntoCell (cellName: string, text: string, textTable: XTextTable) =
    let cell = textTable.getCellByName(cellName)
    let simpleText = cell.query<XSimpleText>()
    simpleText.setString(text)

// Connect to a running office
let context = NativeBootstrap.bootstrap()
// Create a service manager of the remote office
let factory = context.getServiceManager()

// Create the Desktop
let desktop = Desktop.create(context)

// Open a new empty writer document
let componentLoader = desktop.query<XComponentLoader>()
let component_ = componentLoader.loadComponentFromURL(
    "private:factory/swriter", "_blank", 0, [||])

let textDocument = component_.query<XTextDocument>()

// Create a text object
let text = textDocument.getText()
let simpleText = text.query<XSimpleText>()

// Create a cursor object
let cursor = simpleText.createTextCursor()

// Inserting some Text
text.insertString(cursor, "The first line in the newly created text document." + Environment.NewLine, false)

// Create instance of a text table with 4 columns and 4 rows
let textTableI = textDocument.query<XMultiServiceFactory>().createInstance("com.sun.star.text.TextTable")
let textTable = textTableI.query<XTextTable>()
textTable.initialize(4, 4)
text.insertTextContent(cursor, textTable, false)

// Set the table background color
let tablePropertySet = textTableI.query<XPropertySet>()
tablePropertySet.setPropertyValue("BackTransparent", new Any(false))
tablePropertySet.setPropertyValue("BackColor", new Any(0xCCCCFF))

// Get first row
let tableRows = textTable.getRows()
let rowAny = tableRows.query<XIndexAccess>().getByIndex(0)

// Set a different background color for the first row
let firstRowPropertySet = rowAny.cast<XPropertySet>()
firstRowPropertySet.setPropertyValue("BackTransparent", new Any(false))
firstRowPropertySet.setPropertyValue("BackColor", new Any(0x6666AA))

// Fill the first table row
InsertIntoCell("A1", "FirstColumn", textTable)
InsertIntoCell("B1", "SecondColumn", textTable)
InsertIntoCell("C1", "ThirdColumn", textTable)
InsertIntoCell("D1", "SUM", textTable)

// Fill the remaining rows
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

// Change the CharColor and add a Shadow
let cursorPropertySet = cursor.query<XPropertySet>()
cursorPropertySet.setPropertyValue("CharColor", new Any(255))
cursorPropertySet.setPropertyValue("CharShadowed", new Any(true))

// Create a paragraph break
simpleText.insertControlCharacter(cursor, ControlCharacter.PARAGRAPH_BREAK, false)

// Inserting colored Text.
simpleText.insertString(cursor, " This is a colored Text - blue with shadow" + Environment.NewLine, false)

// Create a paragraph break
simpleText.insertControlCharacter(cursor, ControlCharacter.PARAGRAPH_BREAK, false)

// Create a TextFrame.
let textFrameI = textDocument.query<XMultiServiceFactory>().createInstance("com.sun.star.text.TextFrame")
let textFrame = textFrameI.query<XTextFrame>()

// Set the size of the frame
let size = Size(15000, 400)
textFrame.query<XShape>().setSize(size)

// Set anchortype
let framePropertySet = textFrame.query<XPropertySet>()
framePropertySet.setPropertyValue("AnchorType", new Any(TextContentAnchorType.AS_CHARACTER))

// Insert the frame
text.insertTextContent(cursor, textFrame, false)

// Get the text object of the frame
let frameText = textFrame.getText()
let frameSimpleText = frameText.query<XSimpleText>()

// Create a cursor object
let frameCursor = frameSimpleText.createTextCursor()

// Inserting some Text
frameSimpleText.insertString(frameCursor, "The first line in the newly created text frame.", false)
frameSimpleText.insertString(frameCursor, Environment.NewLine + "With this second line the height of the frame raises.", false)

// Create a paragraph break
simpleText.insertControlCharacter(frameCursor, ControlCharacter.PARAGRAPH_BREAK, false)

// Change the CharColor and add a Shadow
cursorPropertySet.setPropertyValue("CharColor", new Any(65536))
cursorPropertySet.setPropertyValue("CharShadowed", new Any(false))

// Insert another string
text.insertString(cursor, Environment.NewLine + " That's all for now !!", false)
