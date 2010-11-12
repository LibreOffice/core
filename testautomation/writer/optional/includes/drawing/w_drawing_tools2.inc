'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
' DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'
' Copyright 2000, 2010 Oracle and/or its affiliates.
'
' OpenOffice.org - a multi-platform office productivity suite
'
' This file is part of OpenOffice.org.
'
' OpenOffice.org is free software: you can redistribute it and/or modify
' it under the terms of the GNU Lesser General Public License version 3
' only, as published by the Free Software Foundation.
'
' OpenOffice.org is distributed in the hope that it will be useful,
' but WITHOUT ANY WARRANTY; without even the implied warranty of
' MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
' GNU Lesser General Public License version 3 for more details
' (a copy is included in the LICENSE file that accompanied this code).
'
' You should have received a copy of the GNU Lesser General Public License
' version 3 along with OpenOffice.org.  If not, see
' <http://www.openoffice.org/license.html>
' for a copy of the LGPLv3 License.
'
'/************************************************************************
'*
'* owner : helge.delfs@oracle.com
'*
'* short description : Test of the drawing tools - group
'*
'\***********************************************************************

sub w_drawing_tools2

    printLog Chr(13) + "- Test of the drawing tools 2 -- Group"
    'Group objects
    Call tGroup_1                'Generate a objects group via menu Format/Group/Group
    Call tGroup_2                'Generate a objects group via contextmenu Format/Group/Group
    Call tGroup_3                'Menu entries may not be enabled if less then two objects are selected
    Call tGroup_4                'Name a selected object group  via Menu Format/Name object
    Call tGroup_5                'Name a selected object group via context menu Name object
    Call tGroup_6                'Edit object group via Menu Format/Group/Edit Group
    Call tGroup_7                'Edit object group via context menu Group/Edit Group
    Call tGroup_8                'Leave object group  via Menu Format/Group/Exit Group
    Call tGroup_9                'Leave object group via context menu Group/Exit Group
    Call tGroup_10               'Create new objects inside the group ,Undo new created objects before leaving the group
    Call tGroup_11               'Create new objects inside the group ,Undo new created objects after leaving the group
    Call tGroup_12               'Save and reload document while editing a group
    Call tEncapsulatedGroup_1    'Group objects inside the group
    
end sub

'-------------------------------------------------------------------------

testcase tGroup_1

    PrintLog "- Generate a objects group via menu Format/Group/Group"

    Call hNewDocument

    PrintLog "- Insert 2 drawing objects (rectangle and ellipse)"
    Call wZeichenobjektEinfuegen("RECHTECK", 32, 30, 50, 65, false)
    Sleep 1
    Call wZeichenobjektEinfuegen("ELLIPSE",  40, 45, 60, 70, false)
    Sleep 1
    Call wObjektSelektieren ( 30, 28, 70, 80 )
    Sleep 1
    
      PrintLog "- via menu Format/Group/Group objects."
    try
        FormatGroupGroup
    catch
        Warnlog "Something wrong when creating group via menu - Format/Group/Group."
    endcatch
    
    Call hCloseDocument

endcase

'-------------------------------------------------------------------------

testcase tGroup_2

    PrintLog "- Generate a objects group via contextmenu Format/Group/Group"

    Call hNewDocument

      PrintLog "- Insert 2 drawing objects (rectangle and ellipse)"
    Call wZeichenobjektEinfuegen("RECHTECK", 32, 30, 50, 65, false)
    Sleep 1
    Call wZeichenobjektEinfuegen("ELLIPSE",  40, 45, 60, 70, false)
    Sleep 1
    Call wObjektSelektieren ( 30, 28, 70, 80 )
    Sleep 1
    
      PrintLog "- via contextmenu Group/Group objects."
    try
        Call wOpenContextMenu
        Call hMenuSelectNr(10)
        Call hMenuSelectNr(1)
    catch
        Warnlog "Something wrong when creating group via contextmenu Group/Group"
    endcatch
    
    Call hCloseDocument

endcase

'-------------------------------------------------------------------------

testcase tGroup_3

    PrintLog "- Menu entries may not be enabled if less then two objects are selected."

    Call hNewDocument

    PrintLog "- Insert 2 drawing objects (rectangle and ellipse)"
    Call wZeichenobjektEinfuegen("RECHTECK", 30, 30, 50, 65, false)
    Sleep 1
    Call wZeichenobjektEinfuegen("ELLIPSE",  40, 45, 60, 70, false)
    Sleep 1
    
      PrintLog "- Only select 1 object."
    Call gMouseClick(32,32)
    Sleep 1
    
      PrintLog "- via menu Format/Group/Group objects."
    try
        FormatGroupGroup
        Warnlog "Should NOT generate the group when less then two objects are selected!"
    catch
    endcatch
    
    Call hCloseDocument

endcase

'-------------------------------------------------------------------------

testcase tGroup_4

    Dim sGroupName as String

    sGroupName = "myTestGroup"

    PrintLog "- Name a selected object group  via Menu Format/Name object"

    Call hNewDocument

    PrintLog "- Insert 2 drawing objects (rectangle and ellipse)"
    Call wZeichenobjektEinfuegen("RECHTECK", 32, 30, 50, 65, false)
    Sleep 1
    Call wZeichenobjektEinfuegen("ELLIPSE",  40, 45, 60, 70, false)
    Sleep 1
    Call wObjektSelektieren ( 30, 28, 70, 80 )
    Sleep 1
    
      PrintLog "- via menu Format/Group/Group objects."
    FormatGroupGroup
    Sleep 1
    
    try
        FormatNameObject
        Kontext "NameDlgObject"
        NameField.SetText sGroupName
        NameDlgObject.OK
    catch
        Warnlog "Something wrong when Naming the group"
    endcatch
    
      PrintLog "- let the focus out of the group."
    Call gMouseClick(5,5)

    Call wNavigatorAuswahl( 12 , 1 )
    Sleep 1

    try
        FormatNameObject
        Sleep 1
        Kontext "NameDlgObject"
        if  NameField.GetText <> sGroupName then Warnlog "The group name is wrong !"
        NameDlgObject.Cancel
    catch
        Warnlog "Something wrong when checking the group name"
    endcatch

    Call wNavigatorClose

    Call hCloseDocument

endcase

'-------------------------------------------------------------------------

testcase tGroup_5

    Dim sGroupName as String

    sGroupName = "myTestGroup"

    PrintLog "- Name a selected object group via context menu Name object "

    Call hNewDocument

    PrintLog "- Insert 2 drawing objects (rectangle and ellipse)"
    Call wZeichenobjektEinfuegen("RECHTECK", 32, 30, 50, 65, false)
    Sleep 1
    Call wZeichenobjektEinfuegen("ELLIPSE",  40, 45, 60, 70, false)
    Sleep 1
    Call wObjektSelektieren ( 28, 28, 70, 80 )
    Sleep 1

    PrintLog "- via menu Format/Group/Group objects"
    FormatGroupGroup
    Sleep 1

    try
        Call wOpenContextMenu
        Call hMenuSelectNr(6)
        Kontext "NameDlgObject"
        NameField.SetText sGroupName
        NameDlgObject.OK
    catch
        Warnlog "Something wrong when Naming the group"
    endcatch
    
    PrintLog "- let the focus out of the group."
    Call gMouseClick(95,95)
    
    Call wNavigatorAuswahl( 12 , 1 )
    Sleep 1
    
    try
        Call wOpenContextMenu
        Call hMenuSelectNr(6)
        Kontext "NameDlgObject"
        if  NameField.GetText <> sGroupName then Warnlog "The group name is wrong !"
        NameDlgObject.Cancel
    catch
        Warnlog "Something wrong when checking the group name"
    endcatch
    
    Call wNavigatorClose
    Call hCloseDocument

endcase

'-------------------------------------------------------------------------

testcase tGroup_6

    Dim sPositionX1 as String
    Dim sPositionY1 as String
    Dim sPositionX2 as String
    Dim sPositionY2 as String

    PrintLog "- Edit object group  via Menu Format/Group/Enter Group"

    Call hNewDocument
    
    PrintLog "- Insert 2 drawing objects (rectangle and ellipse) , generate a group."
    Call wZeichenobjektEinfuegen("RECHTECK", 32, 32, 50, 65, false)
    Sleep 1
    Call gMouseClick(34,34)
    Sleep 1
    Call fPositionAndSize("TabPositionAndSizeWriter")
    sPositionX1  =  Horizontalby.Gettext
    sPositionY1  =  Verticalby.Gettext
    TabPositionAndSizeWriter.Cancel
    Sleep 2
    Call gMouseClick(80,80)
    Sleep 1

    Call wZeichenobjektEinfuegen("ELLIPSE",  40, 40, 60, 70, false)
    Sleep 2
    Call gMouseClick(50,50)
    Sleep 1
    Call fPositionAndSize("TabPositionAndSizeWriter")
    sPositionX2  =  Horizontalby.Gettext
    sPositionY2  =  Verticalby.Gettext
    TabPositionAndSizeWriter.Cancel
    Sleep 2
    Call gMouseClick(80,80)

    Call wObjektSelektieren ( 31, 31, 70, 80 )
    Sleep 2
    FormatGroupGroup
    Sleep 1

    PrintLog "- Format/Group/Enter Group."
    FormatGroupEditGroup
    Sleep 1

    PrintLog "- Set the focus to ellipse, and move it."
    Call gMouseClick(50,50)
    Sleep 1
    Call wTypeKeys "<Right>",2

    PrintLog "- Set the focus to rectangle , check if it is moved (Should NOT)"
    Call gMouseClick(41,40)
    Call fPositionAndSize("TabPositionAndSizeWriter")
    if Horizontalby.Gettext <> sPositionX1 then Warnlog "#i106719#Rectangle should NOT be moved: X Not -> "+sPositionX1+"  but -> "+ Horizontalby.Gettext
    if Verticalby.Gettext   <> sPositionY1 then Warnlog "Rectangle should NOT be moved: Y Not -> "+sPositionY1+"  but -> "+ Verticalby.Gettext
    TabPositionAndSizeWriter.Cancel

    PrintLog "- Set the focus to ellipse , check if it is moved (Should)"
    Call wTypeKeys "<TAB>"
    Call fPositionAndSize("TabPositionAndSizeWriter")
    if Horizontalby.Gettext = sPositionX2 then Warnlog "ellipse should be moved"
    if Verticalby.Gettext   = sPositionY2 then Warnlog "ellipse should be moved"
    TabPositionAndSizeWriter.Cancel

    Call hCloseDocument

endcase

'-------------------------------------------------------------------------

testcase tGroup_7

    Dim sPositionX1 as String
    Dim sPositionY1 as String
    Dim sPositionX2 as String
    Dim sPositionY2 as String

    PrintLog "- Edit object group via context menu Group/Enter Group"

    Call hNewDocument
    
    PrintLog "- Insert 2 drawing objects (rectangle and ellipse) , generate a group."
    Call wZeichenobjektEinfuegen("RECHTECK", 32, 32, 50, 65, false)
    Sleep 1
    Call gMouseClick(40,40)
    Sleep 1
    Call fPositionAndSize("TabPositionAndSizeWriter")
    sPositionX1  =  Horizontalby.Gettext
    sPositionY1  =  Verticalby.Gettext
    TabPositionAndSizeWriter.Cancel
    Sleep 2
    Call gMouseClick(80,80)
    Sleep 1
    
    Call wZeichenobjektEinfuegen("ELLIPSE",  40, 40, 60, 70, false)
    Sleep 2
    Call gMouseClick(50,50)
    Call fPositionAndSize("TabPositionAndSizeWriter")
    sPositionX2  =  Horizontalby.Gettext
    sPositionY2  =  Verticalby.Gettext
    TabPositionAndSizeWriter.Cancel
    Sleep 2
    Call gMouseClick(80,80)
    
    Call wObjektSelektieren ( 31, 31, 70, 80 )
    Sleep 2
    FormatGroupGroup
    Sleep 1
    
      PrintLog "- Context menu Group/Enter Group."
    Call wOpenContextMenu
    Call hMenuSelectNr(12)
    Sleep 1
    Call hMenuSelectNr(2)
    Sleep 1
    
      PrintLog "- Set the focus to ellipse , and move it."
    Call gMouseClick(50,50)
    Sleep 1
    Call wTypeKeys "<Right>",2
    
      PrintLog "- Set the focus to rectangle , check if it is moved (Should NOT)."
    Call gMouseClick(40,40)
    Call fPositionAndSize("TabPositionAndSizeWriter")
    if Horizontalby.Gettext <> sPositionX1 then Warnlog "#i106719#Rectangle should NOT be moved: X Not -> "+sPositionX1+"  but -> "+ Horizontalby.Gettext
    if Verticalby.Gettext   <> sPositionY1 then Warnlog "Rectangle should NOT be moved: Y Not -> "+sPositionY1+"  but -> "+ Verticalby.Gettext
    TabPositionAndSizeWriter.Cancel
    
      PrintLog "- Set the focus to ellipse , check if it is moved (Should)."
    Call wTypeKeys "<TAB>"
    Call fPositionAndSize("TabPositionAndSizeWriter")
    if Horizontalby.Gettext = sPositionX2 then Warnlog "ellipse should be moved"
    if Verticalby.Gettext   = sPositionY2 then Warnlog "ellipse should be moved"
    TabPositionAndSizeWriter.Cancel
    
    Call hCloseDocument

endcase

'-------------------------------------------------------------------------

testcase tGroup_8

    Dim sPositionX as String
    Dim sPositionY as String

    PrintLog "- Leave object group  via Menu Format/Group/Exit Group"

    Call hNewDocument
    
    PrintLog "- Insert 2 drawing objects (rectangle and ellipse) , generate a group."
    Call wZeichenobjektEinfuegen("RECHTECK", 32, 32, 50, 65, false)
    Sleep 1
    Call gMouseClick(34,34)
    Sleep 1
    Call fPositionAndSize("TabPositionAndSizeWriter")
    sPositionX = Horizontalby.Gettext
    sPositionY = Verticalby.Gettext
    TabPositionAndSizeWriter.Cancel
    
    Call gMouseClick(80,80)
    Call wZeichenobjektEinfuegen("ELLIPSE",  40, 45, 60, 70, false)
    Sleep 1
    Call wObjektSelektieren ( 28, 28, 70, 80 )
    Sleep 1
    
    FormatGroupGroup
    Sleep 1
    
      PrintLog "- Format/Group/Edit Group."
    FormatGroupEditGroup
    Sleep 1
    
      PrintLog "- Leave group via Format/Group/Exit Group."
    FormatGroupExitGroup   'after this group , the focus should be on the group
    Sleep 1
    
      PrintLog "- Set the focus to ellipse(in fact, the focus is on the group), and move it."
    Call gMouseClick(50,50)
    Sleep 1
    Kontext "DocumentWriter"
    Call wTypeKeys "<Right>",2
    Call wTypeKeys "<Down>"
    
      PrintLog "- Set the focus to rectangle (in fact, the focus is still on the group) ,"
      PrintLog "- check if it is moved . Group is moved , Rectangle is moved ..."
    Call gMouseClick(45,45)
    Call fPositionAndSize("TabPositionAndSizeWriter")
    if Horizontalby.Gettext = sPositionX then Warnlog "Rectangle X should be moved!"
    if Verticalby.Gettext   = sPositionY then Warnlog "Rectangle Y should be moved!"
    TabPositionAndSizeWriter.Cancel
    
    Call hCloseDocument

endcase

'-------------------------------------------------------------------------

testcase tGroup_9

    Dim sPositionX as String
    Dim sPositionY as String

    PrintLog "- Leave object group  via context Menu Format/Group/Exit Group"

    Call hNewDocument
    
    PrintLog "- Insert 2 drawing objects (rectangle and ellipse) , generate a group."
    Call wZeichenobjektEinfuegen("RECHTECK", 32, 32, 50, 65, false)
    Sleep 1
    Call gMouseClick(35,35)
    Sleep 1
    Call fPositionAndSize("TabPositionAndSizeWriter")
    sPositionX = Horizontalby.Gettext
    sPositionY = Verticalby.Gettext
    TabPositionAndSizeWriter.Cancel
    
    Call gMouseClick(80,80)
    Sleep 1
    Call wZeichenobjektEinfuegen("ELLIPSE",  40, 45, 60, 70, false)
    Sleep 1
    Call wObjektSelektieren ( 28, 28, 70, 80 )
    Sleep 1
    FormatGroupGroup
    Sleep 1
    
    PrintLog "- Format/Group/Edit Group."
    FormatGroupEditGroup
    Sleep 1
    
    PrintLog "- Leave group via context menu Group/Exit Group."
    Call gMouseClick(35,50)
    Sleep 1
    Call wOpenContextMenu
    Sleep 2
    call hMenuSelectNr(12)
    Sleep 2
    Call hMenuSelectNr(1)       'after this group , the focus should be on the group
    Sleep 2
    
      PrintLog "- Set the focus to ellipse(in fact, the focus is on the group), and move it."
    Call gMouseClick(50,50)
    Kontext "DocumentWriter"
    Call wTypeKeys "<Right>",2
    Call wTypeKeys "<Down>"
    
      PrintLog "- Set the focus to rectangle (in fact, the focus is still on the group) ,"
      PrintLog "- check if it is moved . Group is moved , Rectangle is moved ..."
    Call gMouseClick(45,45)
    Call fPositionAndSize("TabPositionAndSizeWriter")
    if Horizontalby.Gettext = sPositionX then Warnlog "Rectangle X should be moved!"
    if Verticalby.Gettext   = sPositionY then Warnlog "Rectangle Y should be moved!"
    TabPositionAndSizeWriter.Cancel
    
    Call hCloseDocument

endcase

'-------------------------------------------------------------------------

testcase tGroup_10

    PrintLog "- create new objects inside the group ,"
    PrintLog "  Undo new created objects before leaving the group"

    Call hNewDocument
    
    PrintLog "- Insert 2 drawing objects (rectangle and ellipse) , generate a group "
    Call wZeichenobjektEinfuegen("RECHTECK", 32, 32, 50, 35, false)
    Sleep 1
    Call wZeichenobjektEinfuegen("ELLIPSE",  40, 60, 50, 80, false)
    Sleep 1
    Call wObjektSelektieren ( 30, 30, 55, 85 )
    Sleep 1

    FormatGroupGroup
    Sleep 1

    PrintLog "- insert another object (rectangle) in the group"
    Call wZeichenobjektEinfuegen("RECHTECK", 35, 40, 45, 55)
    Sleep 1

    PrintLog "- Format/Group/Edit Group."
    Call gMouseClick(32,32)
    Sleep 1
    FormatGroupEditGroup
    Sleep 1
    
      PrintLog "- Undo new created object( rectangle) before leaving the group."
    EditUndo
    Sleep 1
    
    Call gMouseClick(35,40)    'set focus in the rectangle , in fact, the rectangle should be undone now.
    Sleep 1
    try
        Call fPositionAndSize("TabPositionAndSizeWriter")
        TabPositionAndSizeWriter.Cancel
        Warnlog "Undo before leaving the group is FAILED!"
    catch
    endcatch
    
    Call hCloseDocument

endcase

'-------------------------------------------------------------------------

testcase tGroup_11

    PrintLog "- create new objects inside the group ,"
    PrintLog "  Undo new created objects after leaving the group"
    Call hNewDocument
    
    PrintLog "- Insert 2 drawing objects (rectangle and ellipse) , generate a group "
    Call wZeichenobjektEinfuegen("RECHTECK", 30, 30, 50, 35, false)
    Sleep 1
    Call wZeichenobjektEinfuegen("ELLIPSE",  30, 60, 50, 80, false)
    Sleep 1
    Call wObjektSelektieren ( 28, 28, 55, 85 )
    Sleep 1
    
    FormatGroupGroup
    Sleep 1
    
    PrintLog "- insert another object (rectangle) in the group"
    Call wZeichenobjektEinfuegen("RECHTECK", 35, 40, 45, 55, false)
    Sleep 2
    PrintLog "- Format/Group/Edit Group"
    Call gMouseClick(32,32)
    Sleep 3
    FormatGroupEditGroup
    Sleep 1
    
    PrintLog "- Leave the group , Format/Group/Exit Group"
    FormatGroupExitGroup
    Sleep 1
    
    PrintLog "- Undo new created object( rectangle) after leaving the group"
    EditUndo
    Sleep 1
    
    Call gMouseClick(37,42)    'set focus in the rectangle , in fact, the rectangle should be undone now.
    Sleep 1
    try
        Call fPositionAndSize("TabPositionAndSizeWriter")
        TabPositionAndSizeWriter.Cancel
        Warnlog "Undo before leaving the group is FAILED!"
    catch
    endcatch
    
    Call hCloseDocument

endcase

'-------------------------------------------------------------------------

testcase tGroup_12

    Dim sDocument as String

    PrintLog "- Edit object group  via Menu Format/Group/Edit Group"

    sDocument = ConvertPath ( gOfficePath +"user\work\tGroup_12.odt")
    if FileExists ( sDocument ) then app.kill ( sDocument )
    
    Call hNewDocument
    
    PrintLog "- Insert 2 drawing objects (rectangle and ellipse) , generate a group "
    Call wZeichenobjektEinfuegen("RECHTECK", 32, 32, 50, 65, false)
    Sleep 1
    Call wZeichenobjektEinfuegen("ELLIPSE",  40, 45, 60, 70, false)
    Sleep 1
    Call wObjektSelektieren ( 30, 30, 70, 80 )
    Sleep 1
    
    FormatGroupGroup
    Sleep 1
    
      PrintLog "- Format/Group/Edit Group"
    FormatGroupEditGroup
    Sleep 1
    
    PrintLog "- Save document"
    Call hFileSaveAsKill(sDocument)
    
    PrintLog "- close document"
    Call hCloseDocument
    
      PrintLog "- Open previous saved document "
    Call hFileOpen( sDocument)
    
    Kontext "DocumentWriter"
    Call gMouseClick(34,34)    'set focus in the group
    
    PrintLog "- try formatGroupEditGroup"
    try
        FormatGroupEditGroup
    catch
        Warnlog "It should be in the group mode!"
    endcatch
    
    PrintLog "- close document"
    Call hCloseDocument

endcase

'-------------------------------------------------------------------------

testcase tEncapsulatedGroup_1

    PrintLog "- Group objects inside the group ,Encapsulated Group"
    
    Call hNewDocument
    
      PrintLog "- Insert 2 drawing objects (rectangle and ellipse) , generate a group "
      
    Call wZeichenobjektEinfuegen("RECHTECK", 32, 32, 50, 65, false)
    Sleep 1
    Call wZeichenobjektEinfuegen("ELLIPSE",  40, 45, 60, 70, false)
    Sleep 1
    Call wObjektSelektieren ( 31, 31, 70, 80 )
    Sleep 1
    
    FormatGroupGroup
    Sleep 1
    
    Call gMouseClick(90,90)    'set focus outside of the group
    
      PrintLog "- insert 2 drawing objects (rectangle and ellipse) in the group"
    Call wZeichenobjektEinfuegen("RECHTECK", 35, 40, 38, 45, false)
    Call wZeichenobjektEinfuegen("ELLIPSE",  40, 40, 58, 45, false, true)
    Call wObjektSelektieren ( 31, 31, 80, 80 )

    Sleep 1
    
    PrintLog "- generate a new group"
    try
        FormatGroupGroup
    catch
        Warnlog "Unable to execute Format / Group"
    endcatch

    PrintLog "- edit the new group"
    try
        FormatGroupEditGroup
    catch
        Warnlog "Unable to execute Group / Enter Group"
    endcatch

    PrintLog "- leave the new group"
    try
        FormatGroupExitGroup
    catch
        Warnlog "Unable to exceute Format / Exit Group"
    endcatch

    PrintLog "- Undo the new group"
    try
        FormatGroupUnGroup
    catch
        Warnlog "Unable to execute Format / Ungroup"
    endcatch
    
    Call hCloseDocument

endcase
