'/*************************************************************************
 '
 '  $RCSfile: WriterDemo.vb,v $
 '
 '  $Revision: 1.2 $
 '
 '  last change: $Author: rt $ $Date: 2004-08-23 13:17:54 $
 '
 '  The Contents of this file are made available subject to the terms of
 '  either of the following licenses
 '
 '         - GNU Lesser General Public License Version 2.1
 '         - Sun Industry Standards Source License Version 1.1
 '
 '  Sun Microsystems Inc., October, 2000
 '
 '  GNU Lesser General Public License Version 2.1
 '  =============================================
 '  Copyright 2000 by Sun Microsystems, Inc.
 '  901 San Antonio Road, Palo Alto, CA 94303, USA
 '
 '  This library is free software; you can redistribute it and/or
 '  modify it under the terms of the GNU Lesser General Public
 '  License version 2.1, as published by the Free Software Foundation.
 '
 '  This library is distributed in the hope that it will be useful,
 '  but WITHOUT ANY WARRANTY; without even the implied warranty of
 '  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 '  Lesser General Public License for more details.
 '
 '  You should have received a copy of the GNU Lesser General Public
 '  License along with this library; if not, write to the Free Software
 '  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 '  MA  02111-1307  USA
 '
 '
 '  Sun Industry Standards Source License Version 1.1
 '  =================================================
 '  The contents of this file are subject to the Sun Industry Standards
 '  Source License Version 1.1 (the "License"); You may not use this file
 '  except in compliance with the License. You may obtain a copy of the
 '  License at http://www.openoffice.org/license.html.
 '
 '  Software provided under this License is provided on an "ASIS" basis,
 '  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 '  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 '  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 '  See the License for the specific provisions governing your rights and
 '  obligations concerning the Software.
 '
 '  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 '
 '  Copyright: 2000 by Sun Microsystems, Inc.
 '
 '  All Rights Reserved.
 '
 '  Contributor(s): _______________________________________
 '
 '
 '******************************************************************/

Option Explicit On
Option Strict On

imports System	
imports System.Collections
imports Microsoft.VisualBasic
imports unoidl.com.sun.star.lang
imports unoidl.com.sun.star.uno
imports unoidl.com.sun.star.bridge
imports uno.util

Module WriterDemo

Sub Main( ByVal args() As String)
'    If args.Length <> 2 Then
'        Console.WriteLine("WriterDemo takes two arguments. A file url to the office" & _
'        "program directory and a connection string.")
'    End If        
'Connect to a running office 
'--------------------------------------------------

'Create a service manager of the remote office
'Dim ht As Hashtable = New Hashtable()
'ht.Add("SYSBINDIR", args(0))
Dim xContext As XComponentContext
'xLocalContext = Bootstrap.defaultBootstrap_InitialComponentContext( _
'    args(0) & "/uno.ini", ht.GetEnumerator())
        
xContext = Bootstrap.bootstrap()

'Dim xURLResolver As XUnoUrlResolver 
'xURLResolver = DirectCast(xLocalContext.getServiceManager(). _
'    createInstanceWithContext("com.sun.star.bridge.UnoUrlResolver", _
'	    xLocalContext), XUnoUrlResolver)
        
'Dim xRemoteContext As XComponentContext 
'xRemoteContext = DirectCast(xURLResolver.resolve( _
'	"uno:socket,host=localhost,port=8100;urp;StarOffice.ComponentContext"), _
'        XComponentContext)

Dim xFactory As XMultiServiceFactory 
xFactory = DirectCast(xContext.getServiceManager(), _
    XMultiServiceFactory)

'Create the Desktop
Dim xDesktop As unoidl.com.sun.star.frame.XDesktop
xDesktop = DirectCast(xFactory.createInstance("com.sun.star.frame.Desktop"), _
    unoidl.com.sun.star.frame.XDesktop)

'Open a new empty writer document
Dim xComponentLoader As unoidl.com.sun.star.frame.XComponentLoader
xComponentLoader = DirectCast(xDesktop, unoidl.com.sun.star.frame.XComponentLoader)
Dim arProps() As unoidl.com.sun.star.beans.PropertyValue = _
    New unoidl.com.sun.star.beans.PropertyValue(){}
Dim xComponent As unoidl.com.sun.star.lang.XComponent
    xComponent = xComponentLoader.loadComponentFromURL( _
   "private:factory/swriter", "_blank", 0, arProps)
Dim xTextDocument As unoidl.com.sun.star.text.XTextDocument
xTextDocument = DirectCast(xComponent, unoidl.com.sun.star.text.XTextDocument)

'Create a text object
Dim xText As unoidl.com.sun.star.text.XText
xText = xTextDocument.getText()

Dim xSimpleText As unoidl.com.sun.star.text.XSimpleText
xSimpleText = DirectCast(xText, unoidl.com.sun.star.text.XSimpleText)

'Create a cursor object
Dim xCursor As unoidl.com.sun.star.text.XTextCursor
xCursor = xSimpleText.createTextCursor()

'Inserting some Text
xText.insertString(xCursor, "The first line in the newly created text document." _ 
    & vbLf, false)

'Create instance of a text table with 4 columns and 4 rows
Dim objTextTable As Object
objTextTable= DirectCast(xTextDocument, unoidl.com.sun.star.lang.XMultiServiceFactory). _
    createInstance("com.sun.star.text.TextTable")
Dim xTextTable As unoidl.com.sun.star.text.XTextTable
xTextTable = DirectCast(objTextTable, unoidl.com.sun.star.text.XTextTable)
xTextTable.initialize(4, 4)
xText.insertTextContent(xCursor, xTextTable, false)

'Set the table background color
Dim xPropertySetTable As unoidl.com.sun.star.beans.XPropertySet
xPropertySetTable = DirectCast(objTextTable, unoidl.com.sun.star.beans.XPropertySet)
xPropertySetTable.setPropertyValue("BackTransparent", New uno.Any(False))
xPropertySetTable.setPropertyValue("BackColor", New uno.Any(&Hccccff))

'Get first row
Dim xTableRows As unoidl.com.sun.star.table.XTableRows
xTableRows = xTextTable.getRows()
Dim anyRow As uno.Any
anyRow = DirectCast(xTableRows, unoidl.com.sun.star.container.XIndexAccess).getByIndex( 0)

'Set a different background color for the first row
Dim xPropertySetFirstRow As unoidl.com.sun.star.beans.XPropertySet
xPropertySetFirstRow = DirectCast(anyRow.Value, unoidl.com.sun.star.beans.XPropertySet)
xPropertySetFirstRow.setPropertyValue("BackTransparent", New uno.Any(False))
xPropertySetFirstRow.setPropertyValue("BackColor", New uno.Any(&H6666AA))

'Fill the first table row
insertIntoCell("A1","FirstColumn", xTextTable)
insertIntoCell("B1","SecondColumn", xTextTable)
insertIntoCell("C1","ThirdColumn", xTextTable)
insertIntoCell("D1","SUM", xTextTable)

'Fill the remaining rows
xTextTable.getCellByName("A2").setValue(22.5)
xTextTable.getCellByName("B2").setValue(5615.3)
xTextTable.getCellByName("C2").setValue(-2315.7)
xTextTable.getCellByName("D2").setFormula("sum <A2:C2>")
    
xTextTable.getCellByName("A3").setValue(21.5)
xTextTable.getCellByName("B3").setValue (615.3)
xTextTable.getCellByName("C3").setValue( -315.7)
xTextTable.getCellByName("D3").setFormula( "sum <A3:C3>")
           
xTextTable.getCellByName("A4").setValue( 121.5)
xTextTable.getCellByName("B4").setValue( -615.3)
xTextTable.getCellByName("C4").setValue( 415.7)
xTextTable.getCellByName("D4").setFormula( "sum <A4:C4>")

'Change the CharColor and add a Shadow
Dim xPropertySetCursor As unoidl.com.sun.star.beans.XPropertySet
xPropertySetCursor = DirectCast(xCursor, unoidl.com.sun.star.beans.XPropertySet)
xPropertySetCursor.setPropertyValue("CharColor", New uno.Any(255))
xPropertySetCursor.setPropertyValue("CharShadowed", New uno.Any(true))

'Create a paragraph break
xSimpleText.insertControlCharacter(xCursor, _
    unoidl.com.sun.star.text.ControlCharacter.PARAGRAPH_BREAK, False)

'Inserting colored Text.
xSimpleText.insertString(xCursor," This is a colored Text - blue with shadow" & vbLf, _
    False)

'Create a paragraph break
xSimpleText.insertControlCharacter(xCursor, _
    unoidl.com.sun.star.text.ControlCharacter.PARAGRAPH_BREAK, False)

'Create a TextFrame.
Dim objTextFrame As Object
objTextFrame = DirectCast(xTextDocument, unoidl.com.sun.star.lang.XMultiServiceFactory). _
    createInstance("com.sun.star.text.TextFrame")

Dim xTextFrame As unoidl.com.sun.star.text.XTextFrame = _
    DirectCast(objTextFrame, unoidl.com.sun.star.text.XTextFrame)

'Set the size of the frame
Dim aSize As unoidl.com.sun.star.awt.Size = _ 
    New unoidl.com.sun.star.awt.Size(15000, 400)
DirectCast(xTextFrame, unoidl.com.sun.star.drawing.XShape).setSize(aSize)

'Set anchortype
Dim xPropertySetFrame As unoidl.com.sun.star.beans.XPropertySet
xPropertySetFrame = DirectCast(xTextFrame, unoidl.com.sun.star.beans.XPropertySet)
xPropertySetFrame.setPropertyValue("AnchorType", New uno.Any( _ 
    GetType(unoidl.com.sun.star.text.TextContentAnchorType), _
    unoidl.com.sun.star.text.TextContentAnchorType.AS_CHARACTER))

'insert the frame
xText.insertTextContent(xCursor, xTextFrame, False)

'Get the text object of the frame

Dim xFrameText As unoidl.com.sun.star.text.XText
xFrameText = xTextFrame.getText()

Dim xFrameSimpleText As unoidl.com.sun.star.text.XSimpleText
xFrameSimpleText = DirectCast(xFrameText, unoidl.com.sun.star.text.XSimpleText)

'Create a cursor object
Dim xFrameCursor As unoidl.com.sun.star.text.XTextCursor
xFrameCursor = xFrameSimpleText.createTextCursor()

'Inserting some Text
xFrameSimpleText.insertString(xFrameCursor, _
    "The first line in the newly created text frame.", False)
xFrameSimpleText.insertString(xFrameCursor, _
    vbLf & "With this second line the height of the frame raises.", False)

'Create a paragraph break
xSimpleText.insertControlCharacter(xFrameCursor, _
    unoidl.com.sun.star.text.ControlCharacter.PARAGRAPH_BREAK, False)

'Change the CharColor and add a Shadow
xPropertySetCursor.setPropertyValue("CharColor", New uno.Any(65536))
xPropertySetCursor.setPropertyValue("CharShadowed", New uno.Any(False))

'Insert another string
xText.insertString(xCursor, vbLf + " That's all for now !!", False)

End Sub

Sub insertIntoCell(sCellName As String,sText As String, _
     xTable As unoidl.com.sun.star.text.XTextTable)
    Dim xCell As unoidl.com.sun.star.table.XCell 
    xCell = xTable.getCellByName(sCellName)

    Dim xSimpleTextCell As unoidl.com.sun.star.text.XSimpleText
    xSimpleTextCell = DirectCast(xCell, unoidl.com.sun.star.text.XSimpleText)

    Dim xCursor As unoidl.com.sun.star.text.XTextCursor
    xCursor = xSimpleTextCell.createTextCursor()

    Dim xPropertySetCursor As unoidl.com.sun.star.beans.XPropertySet
    xPropertySetCursor = DirectCast(xCursor, unoidl.com.sun.star.beans.XPropertySet)

    xPropertySetCursor.setPropertyValue("CharColor", New uno.Any(&Hffffff))
    xSimpleTextCell.insertString(xCursor, sText, False)
End Sub

End Module