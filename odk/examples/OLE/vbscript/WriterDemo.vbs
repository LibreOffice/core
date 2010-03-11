'***********************************************************************
'*
'*  The Contents of this file are made available subject to the terms of
'*  the BSD license.
'*  
'*  Copyright 2000, 2010 Oracle and/or its affiliates.
'*  All rights reserved.
'*
'*  Redistribution and use in source and binary forms, with or without
'*  modification, are permitted provided that the following conditions
'*  are met:
'*  1. Redistributions of source code must retain the above copyright
'*     notice, this list of conditions and the following disclaimer.
'*  2. Redistributions in binary form must reproduce the above copyright
'*     notice, this list of conditions and the following disclaimer in the
'*     documentation and/or other materials provided with the distribution.
'*  3. Neither the name of Sun Microsystems, Inc. nor the names of its
'*     contributors may be used to endorse or promote products derived
'*     from this software without specific prior written permission.
'*
'*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
'*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
'*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
'*  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
'*  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
'*  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
'*  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
'*  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
'*  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
'*  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
'*  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
'*     
'*************************************************************************

'The service manager is always the starting point
'If there is no office running then an office is started up
Set objServiceManager= WScript.CreateObject("com.sun.star.ServiceManager")

'Create the CoreReflection service that is later used to create structs
Set objCoreReflection= objServiceManager.createInstance("com.sun.star.reflection.CoreReflection")

'Create the Desktop
Set objDesktop= objServiceManager.createInstance("com.sun.star.frame.Desktop")

'Open a new empty writer document
Dim args()	
Set objDocument= objDesktop.loadComponentFromURL("private:factory/swriter", "_blank", 0, args)

'Create a text object
Set objText= objDocument.getText

'Create a cursor object
Set objCursor= objText.createTextCursor

'Inserting some Text
objText.insertString objCursor, "The first line in the newly created text document." & vbLf, false
  
'Inserting a second line
objText.insertString objCursor, "Now we're in the second line", false

'Create instance of a text table with 4 columns and 4 rows
Set objTable= objDocument.createInstance( "com.sun.star.text.TextTable")
objTable.initialize 4, 4

'Insert the table
objText.insertTextContent objCursor, objTable, false

'Get first row
Set objRows= objTable.getRows
Set objRow= objRows.getByIndex( 0)

'Set the table background color
objTable.setPropertyValue "BackTransparent", false
objTable.setPropertyValue		     "BackColor", 13421823

'Set a different background color for the first row
objRow.setPropertyValue "BackTransparent", false
objRow.setPropertyValue "BackColor", 6710932

'Fill the first table row
insertIntoCell "A1","FirstColumn", objTable 
insertIntoCell "B1","SecondColumn", objTable
insertIntoCell "C1","ThirdColumn", objTable
insertIntoCell "D1","SUM", objTable

objTable.getCellByName("A2").setValue 22.5
objTable.getCellByName("B2").setValue 5615.3
objTable.getCellByName("C2").setValue -2315.7
objTable.getCellByName("D2").setFormula"sum <A2:C2>"
    
objTable.getCellByName("A3").setValue 21.5
objTable.getCellByName("B3").setValue 615.3
objTable.getCellByName("C3").setValue -315.7
objTable.getCellByName("D3").setFormula "sum <A3:C3>"
           
objTable.getCellByName("A4").setValue 121.5
objTable.getCellByName("B4").setValue -615.3
objTable.getCellByName("C4").setValue 415.7
objTable.getCellByName("D4").setFormula "sum <A4:C4>"
    
'Change the CharColor and add a Shadow
objCursor.setPropertyValue "CharColor", 255
objCursor.setPropertyValue "CharShadowed", true

'Create a paragraph break
'The second argument is a com::sun::star::text::ControlCharacter::PARAGRAPH_BREAK constant
objText.insertControlCharacter objCursor, 0 , false

'Inserting colored Text.
objText.insertString objCursor, " This is a colored Text - blue with shadow" & vbLf, false

'Create a paragraph break ( ControlCharacter::PARAGRAPH_BREAK).
objText.insertControlCharacter objCursor, 0, false 
      
'Create a TextFrame.
Set objTextFrame= objDocument.createInstance("com.sun.star.text.TextFrame")

'Create a Size struct.
Set objSize= createStruct("com.sun.star.awt.Size")
objSize.Width= 15000
objSize.Height= 400
objTextFrame.setSize( objSize)

' TextContentAnchorType.AS_CHARACTER = 1
objTextFrame.setPropertyValue "AnchorType", 1

'insert the frame
objText.insertTextContent objCursor, objTextFrame, false

'Get the text object of the frame
Set objFrameText= objTextFrame.getText


'Create a cursor object
Set objFrameTextCursor= objFrameText.createTextCursor
    
'Inserting some Text
objFrameText.insertString objFrameTextCursor, "The first line in the newly created text frame.", _
                          false    
objFrameText.insertString objFrameTextCursor, _
                          vbLf & "With this second line the height of the frame raises.", false 

'Create a paragraph break
'The second argument is a com::sun::star::text::ControlCharacter::PARAGRAPH_BREAK constant
objFrameText.insertControlCharacter objCursor, 0 , false

'Change the CharColor and add a Shadow
objCursor.setPropertyValue "CharColor", 65536
objCursor.setPropertyValue "CharShadowed", false

'Insert another string
objText.insertString objCursor, " That's all for now !!", false      

On Error Resume Next
If Err Then
  MsgBox "An error occurred"
End If 


Sub insertIntoCell( strCellName, strText, objTable) 
    Set objCellText= objTable.getCellByName( strCellName)
    Set objCellCursor= objCellText.createTextCursor
    objCellCursor.setPropertyValue "CharColor",16777215
    objCellText.insertString objCellCursor, strText, false
End Sub

Function createStruct( strTypeName)
    Set classSize= objCoreReflection.forName( strTypeName)
    Dim aStruct
    classSize.createObject aStruct
    Set createStruct= aStruct
End Function
