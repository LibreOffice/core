'encoding UTF-8  Do not remove or change this line!
'*************************************************************************
'
'  Licensed to the Apache Software Foundation (ASF) under one
'  or more contributor license agreements.  See the NOTICE file
'  distributed with this work for additional information
'  regarding copyright ownership.  The ASF licenses this file
'  to you under the Apache License, Version 2.0 (the
'  "License"); you may not use this file except in compliance
'  with the License.  You may obtain a copy of the License at
'  
'    http://www.apache.org/licenses/LICENSE-2.0
'  
'  Unless required by applicable law or agreed to in writing,
'  software distributed under the License is distributed on an
'  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
'  KIND, either express or implied.  See the License for the
'  specific language governing permissions and limitations
'  under the License.
'
'*************************************************************************

*************
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
