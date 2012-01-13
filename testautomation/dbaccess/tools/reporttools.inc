'encoding UTF-8  Do not remove or change this line!
'**************************************************************
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
'**************************************************************
'*
'* short description : Report Helper Routines for Base tests.
'*
'***************************************************************************************
'*
'* #1 fOpenNewReoprtDesign
'* #1 fCloseReportDesign
'* #1 fCloseReportView
'* #1 fSaveReport
'* #1 fOpenReport
'* #1 fFindREport
'* #1 fOpenReportNavigator
'* #1 fCloseReportNavigator
'* #1 fSwitchPageheader
'* #1 fExecutereport
'* #1 fCloseAddFieldDialog
'* #1 tools_reporttools_InstallExtension
'*
'\***********************************************************************************
'-------------------------------------------------------------------------
function fOpenNewReportDesign()
    '/// open a a new report design from an open database
    '/// <u>parameter:</u>
          
    Kontext "DATABASE"    
	    if ( Database.NotExists(3) ) then
	        fOpenNewReportDesign = false
	        exit function
	    end if

        Database.MouseDown(50,50)
        Database.MouseUp(50,50)
    
    sleep(1)
    
    ViewReports
    NewReportDesign
    
    sleep(5)
    
   ' Kontext "ReportDesign"
   '     ReportDesign.MouseDown(50,50)
   '     ReportDesign.MouseUp(50,50)
    
   ' sleep(1)
        
    fOpenNewReportDesign = true
    
end function
'--------------------------------------------------------------------
function fCloseReportDesign( optional bSave )    
    '/// close an open Report 
    '/// <u>parameter:</u>
    '/// <b><i>optional</i> bSave:</b> if true the Report shall be saved, if false the changes are lost
    
    sleep(1)

    Kontext "ReportDesign"
        ReportDesign.UseMenu
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
    
    fCloseReportDesign = true

end function
'--------------------------------------------------------------------
function fCloseReportView()    
    '/// close an open Report View 
    '/// <u>parameter:</u>    
    
    sleep(1)

    Kontext "DocumentWriter"
        DocumentWriter.UseMenu
        hMenuSelectNr(1) ' the file menu
        hMenuSelectNr(5) ' the Close Window
        
    sleep(1)
    
    fCloseReportView = true

end function
'--------------------------------------------------------------------
function fSaveReport( sReportName as string, optional bCloseReport as boolean )    
    '/// save an open Report with the given name 
    '/// <u>parameter:</u>    
    '/// <b>sReportName:</b> the name under which the Report shall be saved. If the file allready exists, then the file will be overwritten
    '/// <b><i>optional</i> bCloseReport:</b> if true the Report shall be closed after saving, if false Report stay open
    sleep(1)

    Kontext "DocumentWriter"
        DocumentWriter.UseMenu
        hMenuSelectNr(1) ' the file menu
        hMenuSelectNr(6) ' the Save

    Kontext "ReportSaveDialog"
        if FormSaveDialog.exists(3) then
            FormName.setText(sReportName)
            SaveBtn.Click
            'click yes in the overwrite messages box
            Kontext "MessageBox"
                if MessageBox.exists(1) then
                    MessageBox.Yes
                endif
            fSaveReport = true
        else
            fSaveReport = false
        end if
     
     if ( IsMissing( bCloseReport ) ) then
        ' nothing
     else
        call fCloseReportDesign()
     end if
     
end function
'--------------------------------------------------------------------
function fOpenReport(sReportName as string)    
    '/// open a Report with the given name
    '/// <u>parameter:</u>
    '/// <b>sReportName:</b> the name of the Report which shall be open

    if ( fFindReport(sReportName) = true ) then
        printlog "Report found -> open"    
        Kontext "ContainerView"
            OpenForm ' uno-Slot .uno:DB/Open
            sleep(1)
        fOpenReport = true
    else
        printlog "Report not found."
        fOpenReport = false
    end if
    
end function
'--------------------------------------------------------------------
function fOpenReportInDesign(sReportName as string)    
    '/// open a Report with the given name in the design mode
    '/// <u>parameter:</u>
    '/// <b>sReportName:</b> the name of the Report which shall be open

    if ( fFindReport(sReportName) = true ) then
        printlog "Report found -> open"    
        Kontext "ContainerView"
            EditReport ' uno-Slot .uno:DB/Edit
            sleep(1)
        fOpenReportInDesign = true
    else
        printlog "Report not found."
        fOpenReportInDesign = false
    end if
    
end function
'--------------------------------------------------------------------
function fFindReport(sReportName as string)    
    '/// select a Report with the given name
    '/// <u>parameter:</u>
    '/// <b>sReportName:</b> the name of the Report which shall be selected
    
    Dim iNumbersOfReports as integer
	Dim i as integer
	
    Kontext "DATABASE"
    
        ViewReports        
    
        fFindReport = false
    
        if ( Not DatabaseReportsView.exists(1) ) then
            warnlog "The Report tree doesn't exists"            
            exit function
        end if
        
        iNumbersOfReports = DatabaseReportsView.getItemCount()

        ' this select the first entry
        DatabaseReportsView.TypeKeys "<HOME>"
        DatabaseReportsView.TypeKeys "<UP>"
        
        
        for i = 1 to iNumbersOfReports
            
            DatabaseReportsView.TypeKeys "<ADD>"
            'printlog "i = " + i
            'printlog "ReportName.getItemCount = " + ReportTree.getItemCount 
            if DatabaseReportsView.getItemCount >  iNumbersOfReports then
                iNumbersOfReports = DatabaseReportsView.getItemCount()                    
            endif
            'printlog "ReportName.getSeltext = " + ReportTree.getSeltext
            if DatabaseReportsView.getSeltext = sReportName then
                fFindReport = true    
                exit for
            endif
            DatabaseReportsView.TypeKeys "<DOWN>"
        next
        sleep(1)        
    
end function
'--------------------------------------------------------------------
function fOpenReportNavigator     
    '/// open the report navigator
    '/// <u>parameter:</u>
    printlog "open report navigator"    
    Kontext "ReportDesign"     
        ReportDesign.UseMenu
        hMenuSelectNr(3)
        if (hMenuItemIsChecked(5) = false) then
            hMenuSelectNr(5)
        else
            'nothing
        endif
        
    sleep(1)        
        
end function
'--------------------------------------------------------------------
function fCloseReportNavigator     
    '/// open the report navigator
    '/// <u>parameter:</u>
    printlog "open report navigator"    
    Kontext "ReportDesign"     
        ReportDesign.UseMenu
        hMenuSelectNr(3)
        if (hMenuItemIsChecked(5) = true) then
            hMenuSelectNr(5)
        else
            'nothing
        endif
        
    sleep(1)        
        
end function
'--------------------------------------------------------------------
function fExecuteReport     
    '/// execute the report via the menu
    '/// <u>parameter:</u>
    
    Kontext "ReportDesign"     
        ReportDesign.UseMenu
        hMenuSelectNr(2)
        hMenuSelectNr(14)
        
end function
'--------------------------------------------------------------------
function fSwitchPageHeader     
    '/// delete or insert the Page Header via the menu
    '/// <u>parameter:</u>

    Kontext "ReportDesign"     
        ReportDesign.UseMenu
        hMenuSelectNr(2)
        hMenuSelectNr(8)
    
end function    
'--------------------------------------------------------------------
function fCloseAddFieldDialog   
    '/// close the Add Field Dialog via the slot or menu
    '/// <u>parameter:</u>
    
    Kontext "ReportAddField"
        if (ReportAddField.exists(3)) then            
            ReportAddField.close
        else
            ' check if the dialog is really not there
            ' sometimes the focus is wrong. So I try a second way to disable the dialog
            Kontext "ReportDesign"
                ReportDesign.UseMenu
                hMenuSelectNr(3)
                if (hMenuItemIsChecked(3)) then
                    hMenuSelectNr(3)
                else
                    warnlog "there should be the Add field dialog. But there is none."
                endif
        endif
end function
'--------------------------------------------------------------------
function fOpenAddFieldDialog   
    '/// open the Add Field Dialog via the slot or menu
    '/// <u>parameter:</u>
    
    Kontext "ReportAddField"
        if (ReportAddField.exists(3)) then            
            ' do nothing
        else
            Kontext "ReportDesign"
                ReportDesign.UseMenu
                hMenuSelectNr(3)
                if (hMenuItemIsChecked(3)) then
                    hMenuSelectNr(3)
                    ReportDesign.UseMenu
	                hMenuSelectNr(3)    
                    hMenuSelectNr(3)
                    printlog "1"
                else
                    hMenuSelectNr(4)
                    printlog "2"
                endif
            Kontext "ReportAddField"
            ReportAddField
            if (ReportAddField.exists(3)) then            
                ' do nothing
            else
                warnlog "there should be the Add field dialog. But there is none."
            endif
        endif
        
end function
'--------------------------------------------------------------------
function tools_reporttools_InstallExtension
    '/// install the report designer extension
    '/// <u>parameter:</u>
    '/// <u>Return Value:</u><br>

    '/// <ol>
    '/// +<li>Errorcode (Integer)</li>
    '/// <ul>
    '/// <li>0 = No errors, extension was added correctly</li>
    '/// <li> not 0 = an error occure see hExtensionAddGUI for more info </li>
    '/// </ul>
    '/// </ol>
    
    dim i as integer
    use "global/tools/includes/optional/t_extension_manager_tools.inc"
    if (not hIsExtensionAlreadyInstalled("Report Builder")) then
        i = hExtensionAddGUI( gTesttoolPath + "dbaccess/required/input/oracle-report-builder.oxt", "AcceptLicense,InstallForUser" )
        if (i > 0) then    
            tools_reporttools_InstallExtension = 0
        else
            if (i = -5) then
                warnlog "Report extension was not installed due to unsatisfied dependencies."
                tools_reporttools_InstallExtension = 5
            else
                warnlog "Report extension was not installed. Error Code = " + i
                tools_reporttools_InstallExtension = i
            endif
        endif
    else
        tools_reporttools_InstallExtension = 0    
    endif
    
    ' restart the office
    call ExitRestartTheOffice
    
end function
