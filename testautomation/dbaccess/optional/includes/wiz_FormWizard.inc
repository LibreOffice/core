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
'* owner : marc.neumann@oracle.com
'*
'* short description : Form Wizard Test
'*
'\***********************************************************************
sub wiz_FormWizard
	
    call tNewForm
    call tCheckTemplate
    call tStartFromTableContextMenu
    
end sub
'--------------------------------------------------------------------------------------
testcase tNewForm

    call fOpenDatabase(gOfficePath + ConvertPath("user/database/biblio.odb"))

    ViewForms

    Kontext "DATABASE"
    
    Database.MouseDown(50,50)
    Database.MouseUp(50,50)
    sleep(1)
    
    StartFormWizard
    
    sleep(5)
    
    Kontext "FormWizard"
        TablesOrQueriesMaster.select 1
        '/// add the third field
        printlog "add the third field"
        AvailableMasterFields.select 3
        sleep(1)
        MasterMoveSelected.click
        sleep(1)
        '/// click NEXT
        printlog "click NEXT"
        NextBtn.click
        sleep(1)
        '/// click NEXT
        printlog "click NEXT"
        NextBtn.click
        sleep(1)     
        '/// click NEXT
        printlog "click NEXT"
        NextBtn.click
        sleep(1)     
        '/// click NEXT
        printlog "click NEXT"
        NextBtn.click
        sleep(1)     
        '/// click NEXT
        printlog "click NEXT"
        NextBtn.click
        sleep(1)             
        '/// click Finish
        printlog "click Finish"       
        FinishBtn.Click
    
    sleep(10)

    call fCloseForm ' should be changed to a CloseReport

    call fCloseDatabase

endcase
'--------------------------------------------------------------------------------------
testcase tCheckTemplate

    call fOpenDatabase(gOfficePath + ConvertPath("user/database/biblio.odb"))

    ViewForms

    Kontext "DATABASE"
    
    Database.MouseDown(50,50)
    Database.MouseUp(50,50)
    sleep(1)
    
    StartFormWizard
    
    sleep(5)
    
    Kontext "FormWizard"
        TablesOrQueriesMaster.select 1
        '/// add the third field
        printlog "add the third field"
        AvailableMasterFields.select 5
        sleep(1)
        MasterMoveSelected.click
        sleep(1)
        '/// click NEXT
        printlog "click NEXT"
        NextBtn.click
        sleep(1)
        '/// click NEXT
        printlog "click NEXT"
        NextBtn.click
        sleep(1)
        '/// on the arrange tabpage
        printlog "on the arrange tabpage"

        'AlignLeft              HID_DLGFORM_CMDALIGNLEFT
        'AlignRight             HID_DLGFORM_CMDALIGNRIGHT
        
        '/// click on all arragements
        printlog "click on all arragements"

        'FormWizard.TypeKeys("<TAB>",true) 
        'sleep(1)
        'FormWizard.TypeKeys("<LEFT>",true)
        'sleep(1)
        'FormWizard.TypeKeys("<LEFT>",true)
        'sleep(1)
        'FormWizard.TypeKeys("<SPACE>",true)
        'sleep(5)
        'FormWizard.TypeKeys("<RIGHT>",true)
        'sleep(1)
        'FormWizard.TypeKeys("<SPACE>",true)
        'sleep(5)
        'FormWizard.TypeKeys("<RIGHT>",true)
        'sleep(1)
        'FormWizard.TypeKeys("<SPACE>",true)
        'sleep(5)
        'FormWizard.TypeKeys("<RIGHT>",true)
        'sleep(1)
        'FormWizard.TypeKeys("<SPACE>",true)
        'sleep(5)

        'ColumnarLabelsLeft.Click        
        'ColumnarLabelsOnTop.Click
        'AsDataSheet.Click
        'InBlocksLabelsAbove.Click

        'NoBorder               HID_DLGFORM_CMDNOBORDER
        'ThreeDLook             HID_DLGFORM_CMD3DBORDER
        'FlatLook               HID_DLGFORM_CMDSIMPLEBORDER

        '/// click NEXT
        printlog "click NEXT"
        NextBtn.click
        sleep(1)     
        '/// click NEXT
        printlog "click NEXT"
        NextBtn.click
        sleep(1)
        ' page styles
        
        dim iCount as integer
        dim i as integer
        
        iCount = PageStyles.getItemCount

        if iCount <> 10 then
            warnlog "there sould be 10 page styles but there are " + iCount + " page styles."
        endif

        for i = 1 to iCount
            PageStyles.select i
            sleep(1)
            '/// check all field border styles
            NoBorder.Check
            sleep(1)
            ThreeDLook.Check
            sleep(1)
            FlatLook.Check
            sleep(1)
        next
     
        '/// click NEXT
        printlog "click NEXT"
        NextBtn.click
        sleep(1)             
        '/// click Finish
        printlog "click Finish"       
        FinishBtn.Click
    
    sleep(10)

    call fCloseForm ' should be changed to a CloseReport

    call fCloseDatabase

endcase
'--------------------------------------------------------------------------------------
testcase tStartFromTableContextMenu

    call fOpenDatabase(gOfficePath + ConvertPath("user/database/biblio.odb"))
        
    Kontext "DATABASE"
    
    Database.MouseDown(50,50)
    Database.MouseUp(50,50)
    sleep(1)

    '/// select the table biblio
    printlog "select the table biblio"
    call fFindTable("biblio")

    Kontext "ContainerView"

    '/// select FormWizard from the table contextmenu
    printlog "select FormWizard from the table contextmenu"
    ContainerView.TypeKeys("<SHIFT F10>",true)
    hMenuSelectNr(6)
    
    '/// check if the formwizard appear
    printlog "check if the formwizard appear"
    Kontext "FormWizard"
        'wait until the FormWizard is started        
        if (FormWizard.exists(5)) then
            '/// cancel the formwizard
            printlog "cancel the formwizard"       
            CancelBtn.Click
        else
            warnlog "The formwizard doesn't appear"
        endif
    
    sleep(1)

    call fCloseDatabase

endcase
'--------------------------------------------------------------------------------------

