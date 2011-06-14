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
'* short description : Helper Routines for Base tests.
'*
'***************************************************************************************
'*
'* #1 fOpenNewFormDesign
'* #1 fCloseForm
'* #1 fSaveForm
'* #1 fOpenForm
'* #1 fFindForm
'*
'\***********************************************************************************
'-------------------------------------------------------------------------
function fOpenNewFormDesign()
    '/// open a a new form design from an open database
    '/// <u>parameter:</u>
          
    Kontext "DATABASE"    
	    if ( Database.NotExists(3) ) then
	        fOpenNewFormDesign = false
	        exit function
	    end if

        Database.MouseDown(50,50)
        Database.MouseUp(50,50)
    
    sleep(1)
    
    ViewForms
    NewFormDesign
    
    sleep(2)
    
    fOpenNewFormDesign = true
    
end function
'--------------------------------------------------------------------
function fCloseForm( optional bSave )    
    '/// close an open form 
    '/// <u>parameter:</u>
    '/// <b><i>optional</i> bSave:</b> if true the form shall be saved, if false the changes are lost
    
    sleep(1)

    Kontext "DocumentWriter"
        DocumentWriter.UseMenu
        hMenuSelectNr(1) ' the file menu
        hMenuSelectNr(5) ' the Close Window
    
    'when issue 30401 is fixed this has to be changed            
    Kontext "Messagebox"        
        if Messagebox.Exists(3) then
            if ( IsMissing( bSave ) ) then
                Messagebox.No
            else
                if bSave then
                    Messagebox.Yes
                else
                    Messagebox.No
                endif
            endif
        end if        
    
    sleep(1)
    
    fCloseForm = true

end function
'--------------------------------------------------------------------
function fSaveForm( sFormName as string, optional bCloseForm as boolean )    
    '/// save an open form with the given name 
    '/// <u>parameter:</u>    
    '/// <b>sFormName:</b> the name under which the form shall be saved. If the file allready exists, then the file will be overwritten
    '/// <b><i>optional</i> bCloseForm:</b> if true the form shall be closed after saving, if false form stay open
    sleep(1)

    Kontext "DocumentWriter"
        DocumentWriter.UseMenu
        hMenuSelectNr(1) ' the file menu
        hMenuSelectNr(6) ' the Save

    Kontext "FormSaveDialog"
        if FormSaveDialog.exists(3) then
            FormName.setText(sFormName)
            SaveBtn.Click
            'click yes in the overwrite messages box
            Kontext "MessageBox"
                if MessageBox.exists(1) then
                    MessageBox.Yes
                endif
            fSaveForm = true    
        else
            fSaveForm = false
        end if
     
     if ( IsMissing( bCloseForm ) ) then
        ' nothing
     else
        call fCloseForm()
     end if
     
end function
'--------------------------------------------------------------------
function fOpenForm(sFormName as string)    
    '/// open a form with the given name
    '/// <u>parameter:</u>
    '/// <b>sFormName:</b> the name of the form which shall be open

    if ( fFindForm(sFormName) = true ) then
        printlog "Form found -> open"    
        Kontext "ContainerView"
            OpenForm ' uno-Slot .uno:DB/Open
            sleep(1)
        fOpenForm = true
    else
        printlog "Form not found."
        fOpenForm = false
    end if
    
end function
'--------------------------------------------------------------------
function fFindForm(sFormName as string)    
    '/// select a form with the given name
    '/// <u>parameter:</u>
    '/// <b>sFormName:</b> the name of the form which shall be selected
    
    Dim iNumbersOfForms as integer
	Dim i as integer
	
    Kontext "ContainerView"
    
        ViewForms        
    
        fFindForm = false
    
        if ( Not FormTree.exists(1) ) then
            warnlog "The form tree doesn't exists"            
            exit function
        end if
        
        iNumbersOfForms = FormTree.getItemCount()

        ' this select the first entry
        FormTree.TypeKeys "<HOME>"
        FormTree.TypeKeys "<UP>"
        
        
        for i = 1 to iNumbersOfForms
            
            FormTree.TypeKeys "<ADD>"
            'printlog "i = " + i
            'printlog "FormName.getItemCount = " + FormTree.getItemCount 
            if FormTree.getItemCount >  iNumbersOfForms then
                iNumbersOfForms = FormTree.getItemCount()                    
            endif
            'printlog "FormName.getSeltext = " + FormTree.getSeltext
            if FormTree.getSeltext = sFormName then
                fFindForm = true    
                exit for
            endif
            FormTree.TypeKeys "<DOWN>"
        next
        sleep(1)        
    
end function
