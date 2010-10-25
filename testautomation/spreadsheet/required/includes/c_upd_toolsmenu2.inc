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
'/***********************************************************************
'*
'* owner : oliver.craemer@oracle.com
'*
'* short description : Resource Test - Tools Menu Part II
'*
'************************************************************************
'*
' #1 tToolsMacro
' #1 tToolsExtensionManager
' #1 tToolsXMLFilterSettings
' #1 tToolsCustomize
' #1 tToolsOptions
' #1 tToolsSolver
'*
'\***********************************************************************

sub c_upd_toolsmenu2

    Printlog Chr(13) + "--------- Tools Menu Part II (c_upd_toolsmenu2.inc) ---------"
    

    call tToolsMacro
    call tToolsExtensionManager
    call tToolsXMLFilterSettings
    call tToolsCustomize
    call tToolsOptions
    call tToolsSolver

end sub

'-----------------------------------------------------------

testcase tToolsMacro
'///<u><b>Tools - Macro</b></u>

	Dim iWhichSecurityLevel as integer
	printlog " Set SecurityLevel to low and remember starting value"
	iWhichSecurityLevel = hSetMacroSecurity(0)

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"
    Call hNewDocument
    '/// Record macro by 'Tools – Macro – Record'
    printlog " Record macro by 'Tools – Macro – Record'"
'    ToolsMacroRecordMacro
'    Kontext "
    '/// Close macro recording by clicking 'Stop Recording'
    printlog " Close macro recording by clicking 'Stop Recording'"
'    
    '/// Open 'Macro Selector' dialog by 'Tools – Macro – Run'
    printlog " Open 'Macro Selector' dialog by 'Tools – Macro – Run'"
    ToolsMacrosRunMacro
    Kontext "ScriptSelector"
    Call DialogTest ( ScriptSelector )
    '/// Close 'Macro Selector' dialog with 'Cancel
    printlog " Close 'Macro Selector' dialog with 'Cancel"
    ScriptSelector.Cancel
    '/// Open Macro dialog by 'Tools – Macro – Organize Macros – StarOffice Basic'
    printlog " Open Macro dialog by 'Tools – Macro – Organize Macros – StarOffice Basic'"
    ToolsMacro
    Kontext "Makro"
    Call DialogTest ( Makro )
    '/// Click on 'Assign' button
    printlog " Click on 'Assign' button"
    Zuordnen.Click
    Kontext
    Active.SetPage TabCustomizeMenu
    Kontext "TabCustomizeMenu"
    '/// Close 'Customize' dialog with 'Cancel'
    printlog " Close 'Customize' dialog with 'Cancel'"
    TabCustomizeMenu.Cancel
    Kontext "Makro"
    '/// Click on 'Edit' button
    printlog " Click on 'Edit' button"
    Bearbeiten.Click
    '/// Close basic window by window closer
    printlog " Close basic window by window closer"
    Kontext "BasicIDE"
    Call hCloseDocument
    '/// Reopen Macro dialog by 'Tools – Macro – Organize Macros – StarOffice Basic'
    printlog " Reopen Macro dialog by 'Tools – Macro – Organize Macros – StarOffice Basic'"
    ToolsMacro
    Kontext "Makro"
    '/// Click on 'Delete' button
    printlog " Click on 'Delete' button"
    Loeschen.Click
    '/// Close messagebox by 'No'
    printlog " Close messagebox by 'No'"
    Kontext
    if Active.exists then 
        Active.no
    end if
    Kontext "Makro"
    '/// Click on Organizer' button
    printlog " Click on Organizer' button"
    Verwalten.Click
    Kontext
    Active.SetPage TabModule
    Kontext "TabModule"
    '/// Close 'Organizer with 'Close'
    printlog " Close 'Organizer with 'Close'"
    TabModule.Close
    Kontext "Makro"
    '/// Close macro dialog
    printlog " Close macro dialog"
    Makro.Close
    '/// Open 'JavaScript Macros' dialog by 'Tools – Macro – Organize Macros – JavaScript'
    printlog " Open 'JavaScript Macros' dialog by 'Tools – Macro – Organize Macros – JavaScript'"
    ToolsMacrosOrganizeMacrosJavaScript
    Kontext "ScriptOrganizer"
    '/// Close 'JavaScript Macros' dialog by 'Close'
    printlog " Close 'JavaScript Macros' dialog by 'Close'"
    ScriptOrganizer.Close
    '/// Open 'BeanShell Macros' dialog by 'Tools – Macro – Organize Macros –  BeanShell'
    printlog " Open 'BeanShell Macros' dialog by 'Tools – Macro – Organize Macros –  BeanShell'"
    ToolsMacrosOrganizeMacrosBeanShell
    Kontext "ScriptOrganizer"
    '/// Close ' BeanShell Macros' dialog by 'Close'
    printlog " Close ' BeanShell Macros' dialog by 'Close'"
    ScriptOrganizer.Close
    Call hCloseDocument
    printlog " Open testdocument 'gTesttoolPath + spreadsheet\required\input\DocwithMacro.ods'"
    Call hFileOpen (Convertpath (gTesttoolPath + "spreadsheet\required\input\DocwithMacro.ods" ))
    Kontext
    if Active.exists then 
        Active.ok
    end if
    Printlog " Save document as 'user/work/DocwithMacro.ods'"
    Call hFileSaveAsKill convertpath(gOfficePath & "user/work/DocwithMacro.ods")
    printlog " Use 'Tools – Macro – Signature'"
    ToolsMacrosSignature
    Kontext "DigitalSignature"
    Call DialogTest ( DigitalSignature )
    '/// On 'Digital Signatures' dialog press 'Add' button
    printlog " On 'Digital Signatures' dialog press 'Add' button"
    Add.Click
    Kontext "SelectCertificate"
    Call DialogTest ( SelectCertificate )
    '/// Close Select Certificate' dialog with 'Cancel'
    printlog " Close Select Certificate' dialog with 'Cancel'"
    SelectCertificate.Cancel
    Kontext "DigitalSignature"
    '/// Close 'Digital Signatures' dialog with 'Cancel'
    printlog " Close 'Digital Signatures' dialog with 'Cancel'"
    DigitalSignature.ok
    '/// Open 'Macro Organizer' by 'Tools – Macro – Organize Dialogs'
    printlog " Open 'Macro Organizer' by 'Tools – Macro – Organize Dialogs'"
    ToolsMacrosOrganizeDialogs
    '/// Switch to tabpage 'Modules'
    printlog " Switch to tabpage 'Modules'"
    Kontext
    Active.SetPage TabModule
    Kontext "TabModule"
    Call DialogTest ( TabModule )
    '/// Switch to tabpage 'Dialogs'
    printlog " Switch to tabpage 'Dialogs'"
    Kontext
    Active.SetPage TabDialogs
    Kontext "TabDialogs"
    Call DialogTest ( TabDialogs )
    '/// Switch to tabpage 'Libraries'
    printlog " Switch to tabpage 'Libraries'"
    Kontext
    Active.SetPage TabBibliotheken
    Kontext "TabBibliotheken"
    Call DialogTest ( TabBibliotheken )
    '/// Close dialog with 'Close'
    printlog " Close dialog with 'Close'"
    TabBibliotheken.Close
    '/// Close document
    printlog " Close document"
    Call hCloseDocument
    printlog " Reset SecurityLevel to starting value"
    Call hSetMacroSecurity(iWhichSecurityLevel)

endcase

'-----------------------------------------------------------

testcase tToolsExtensionManager
'///<u><b>Tools - Extension Manager</b></u>

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"
    Call hNewDocument
    '/// Open 'Extension Manager' by 'Tools – Extension Manager'
    printlog " Open 'Extension Manager' by 'Tools – Extension Manager'"
    ToolsPackagemanager
    Kontext "PackageManager"
    Call DialogTest ( PackageManager )
    '/// Close 'Extension Manager' with 'Close'
    printlog " Close 'Extension Manager' with 'Close'"
    PackageManager.Close
    '/// Close document
    printlog " Close document"
    Call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tToolsXMLFilterSettings
'///<u><b>Tools - XML Filter Settings</b></u>

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"
    Call hNewDocument
    '/// Open 'XML Filter Settings' dialog by 'Tools – XML Filter Settings'
    printlog " Open 'XML Filter Settings' dialog by 'Tools – XML Filter Settings'"
    ToolsXMLFilterSettings
    Kontext "XMLFilterSettings"
    Call DialogTest ( XMLFilterSettings )
    '/// Close dialog with 'Close'
    printlog " Close dialog with 'Close'"
    CloseBtn.Click
    '/// Close document
    printlog " Close document"
    Call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tToolsCustomize
'///<u><b>Tools - Customize</b></u>

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"
    Call hNewDocument
    '/// Open 'Customize' dialog by 'Tools – Customize'
    printlog " Open 'Customize' dialog by 'Tools – Customize'"
    ToolsCustomize
    '/// Switch to tabpage 'Menus'
    printlog " Switch to tabpage 'Menus'"
    Kontext
    Active.SetPage TabCustomizeMenu
    Kontext "TabCustomizeMenu"
    Call DialogTest ( TabCustomizeMenu )
    '/// Switch to tabpage 'Keyboard'
    printlog " Switch to tabpage 'Keyboard'"
    Kontext
    Active.SetPage TabTastatur
    Kontext "TabTastatur"
    Call DialogTest ( TabTastatur )
    '/// Switch to tabpage 'Toolbars'
    printlog " Switch to tabpage 'Toolbars'"
    Kontext
    Active.SetPage TabCustomizeToolbars
    Kontext "TabCustomizeToolbars"
    Call DialogTest ( TabCustomizeToolbars )
    '/// Switch to tabpage 'Events'
    printlog " Switch to tabpage 'Events'"
    Kontext
    Active.SetPage TabCustomizeEvents
    Kontext "TabCustomizeEvents"
    Call DialogTest ( TabCustomizeEvents )
    '/// Close 'Customize' dialog with 'Cancel'
    printlog " Close 'Customize' dialog with 'Cancel'"
    TabCustomizeEvents.Cancel
    '/// Close document
    printlog " Close document"
    Call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tToolsOptions
'///<u><b>Tools - Options</b></u>

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"
    Call hNewDocument
    '/// Open 'Options' dialog by 'Tools – Options'
    printlog " Open 'Options' dialog by 'Tools – Options'"
    ToolsOptions
    '/// Choose 'Calc – General'
    printlog " Choose 'Calc – General'"
    hToolsOptions("CALC", "GENERAL")
    Call DialogTest ( TabLayoutCalc )
    '/// Verify that the 'Measurement' combobox has items
    printlog " Verify that the 'Measurement' combobox has items"
    if Masseinheit.GetItemCount < 1 then
        warnlog "The listbox is empty"
    end if
    '/// Increase 'Tab stops'
    printlog " Increase 'Tab stops'"
    Tabulator.More
    '/// Verify that the 'Move selection' combobox has four items
    printlog " Verify that the 'Move selection' combobox has four items"
    If SelektionNach.GetItemCount <> 4 then
        warnlog "There are not 4 moves possible"
    end if
    '/// Choose 'Calc – View'
    printlog " Choose 'Calc – View'"
    hToolsOptions("CALC", "VIEW")
    Call DialogTest ( TabInhaltCalc )
    '/// Verify that the 'Color' combobox has items
    printlog " Verify that the 'Color' combobox has items"
    if Farbe.GetItemCount < 1 then
        warnlog "The listbox is empty"
    end if
    '/// Verify that the 'Objects' combobox has items
    printlog " Verify that the 'Objects' combobox has items"
    if ObjekteGrafiken.GetItemCount < 1 then
        warnlog "The listbox is empty"
    end if
    '/// Verify that the 'Charts' combobox has items
    printlog " Verify that the 'Charts' combobox has items"
    if Diagramme.GetItemCount < 1 then
        warnlog "The listbox is empty"
    end if
    '/// Verify that the 'Drawing objects' combobox has items
    printlog " Verify that the 'Drawing objects' combobox has items"
    if Zeichenobjekte.GetItemCount < 1 then
        warnlog "The listbox is empty"
    end if
    '/// Choose 'Calc – Calculate'
    printlog " Choose 'Calc – Calculate'"
    hToolsOptions("CALC", "CALCULATE")
    Call DialogTest ( TabBerechnen )
    '/// Choose 'Calc – Sortlists'
    printlog " Choose 'Calc – Sortlists'"
    hToolsOptions("CALC", "SORTLISTS")
    Call DialogTest ( TabSortierlisten )
    '/// Verify that there are sortlists available
    printlog " Verify that there are sortlists available"
    if Listen.GetItemCount < 1 then
        warnlog "The listbox is empty"
    end if
    '/// Choose 'Calc – Changes'
    printlog " Choose 'Calc – Changes'"
    hToolsOptions("CALC", "CHANGES")
    Call DialogTest ( TabAenderungenCalc )
    '/// Verify that the 'Changes' combobox has items
    printlog " Verify that the 'Changes' combobox has items"
    if Aenderungen.GetItemCount < 1 then
        warnlog "The listbox is empty"
    end if
    '/// Choose 'Calc – Grid'
    printlog " Choose 'Calc – Grid'"
    hToolsOptions("CALC", "GRID")
    Call DialogTest ( TabRaster )
    '/// Decrease 'Horizontal Resolution'
    printlog " Decrease 'Horizontal Resolution'"
    RasterAufloesungXAchse.Less
    '/// Increase 'Horizontal Subdivision'
    printlog " Increase 'Horizontal Subdivision'"
    RasterUnterteilungXAchse.More
    '/// Choose 'Calc – Print'
    printlog " Choose 'Calc – Print'"
    hToolsOptions("CALC", "PRINT")
    Call DialogTest ( TabPrintCalcOptions )
    '/// Close 'Options' dialog with 'Cancel'
    printlog " Close 'Options' dialog with 'Cancel'"
    Kontext "OptionenDlg"
    OptionenDlg.Cancel
    '/// Close document
    printlog " Close document"
    Call hCloseDocument

endcase

'-----------------------------------------------------------

testcase tToolsSolver
'///<u><b>Tools - Solver</b></u>

    Dim i as integer

    '/// Open new Spreadsheet document
    printlog " Open new Spreadsheet document"
    Call hNewDocument
    '/// Open 'Solver' dialog by 'Tools - Solver'
    printlog " Open 'Solver' dialog by 'Tools - Solver'"
    try
        ToolsSolver
    catch
        qaerrorlog "Solver not available. This is true for versions older than SRC680_m246"
        call hCloseDocument
        goto endsub
    endcatch
    Kontext "Solver"
    DialogTest ( Solver )
    '/// Enter A1 for 'TargetCell'
    printlog " Enter A1 for 'TargetCell'"
    TargetCell.SetText "A1" 
    '/// Press 'Shrink' button for 'Target cell'
    printlog " Press 'Shrink' button for 'Target cell'"
    Shrink.Click
    '/// On minimized dialog press 'Shrink' button again
    printlog " On minimized dialog press 'Shrink' button again"
    Shrink.Click
    '/// Select 'Maximum'
    printlog " Select 'Maximum'"
    OptimizeMaximum.Check
    '/// Select 'Minimum'
    printlog " Select 'Minimum'"
    OptimizeMinimum.Check
    '/// Select 'Value of'
    printlog " Select 'Value of'"
    OptimizeValueOf.Check
    '/// Minimize dialog by pressing 'Shrink' button for 'Value of'
    printlog " Minimize dialog by pressing 'Shrink' button for 'Value of'"
    OptimizeValueEditShrink.Click
    '/// On minimized dialog press 'Shrink' button again
    printlog " On minimized dialog press 'Shrink' button again"
    OptimizeValueEditShrink.Click
    '/// Minimize dialog by pressing 'Shrink' button for 'By changing cells'
    printlog " Minimize dialog by pressing 'Shrink' button for 'By changing cells'"
    ChangingCellsShrink.Click
    '/// On minimized dialog press 'Shrink' button again
    printlog " On minimized dialog press 'Shrink' button again"
    ChangingCellsShrink.Click
    '/// Enter 'A2' in edit box for 'Cell reference 1'
    printlog " Enter 'A2' in edit box for 'Cell reference 1'"
    CellReference1.SetText "A2"
    '/// Minimize dialog by pressing 'Shrink' button for 'Cell reference 1'
    printlog " Minimize dialog by pressing 'Shrink' button for 'Cell reference 1'"
    CellReferenceShrink1.Click
    '/// On minimized dialog press 'Shrink' button again
    printlog " On minimized dialog press 'Shrink' button again"
    CellReferenceShrink1.Click
    '/// Verify that there are 'Operators' available
    printlog " Verify that there are 'Operators' available"
    If Operator1.GetItemCount < 1 then
        warnlog "There are no operators available"
    end if
    '/// Minimize dialog by pressing 'Shrink' button for 'Value 1'
    printlog " Minimize dialog by pressing 'Shrink' button for 'Value 1'"
    ValueShrink1.Click
    '/// On minimized dialog press 'Shrink' button again
    printlog " On minimized dialog press 'Shrink' button again"
    ValueShrink1.Click
    '/// Press 'Remove' button
    printlog " Press 'Remove' button"
    Remove1.Click
    '/// Press 'Options' button
    printlog " Press 'Options' button"
    Options.Click
    Kontext "SolverOptions"
    DialogTest ( SolverOptions )
    '/// On 'Options' dialog verify that there is min 1 'Solver engine' available
    printlog " On 'Options' dialog verify that there is min 1 'Solver engine' available"
    If SolverEngine.GetItemCount < 1 then
        warnlog "There is no SolverEngine available"
    end if
    '/// Check for 'Settings' which enables the edit button (e.g.Epsilon level)
    printlog " Check for 'Settings' which enables the edit button (e.g.Epsilon level)"
    For i = 1 to SolverSettings.GetItemCount
        SolverSettings.Select i
        if SolverSettingsEdit.IsEnabled then
            i = SolverSettings.GetItemCount
        end if
    next i
    If not SolverSettingsEdit.IsEnabled then
        warnlog "Edit is not enabled"
        SolverOptions.Cancel
        Kontext "Solver"
        Solver.Close
        call hCloseDocument
        goto endsub
    else
        '/// Click 'Edit' button to open 'Edit Setting' dialog
        printlog " Click 'Edit' button to open 'Edit Setting' dialog"
        SolverSettingsEdit.Click
        Kontext "SolverEditSetting"
        DialogTest ( SolverEditSetting )
    end if
    '/// Increase 'Epsilon level'
    printlog " Increase 'Epsilon level'"
    SolverNumeric.More
    '/// Close 'Edit Setting' dialog by 'Cancel'
    printlog " Close 'Edit Setting' dialog by 'Cancel'"
    SolverEditSetting.Cancel
    Kontext "SolverOptions"
    '/// Close 'Options' dialog by 'Cancel'
    printlog " Close 'Options' dialog by 'Cancel'"
    SolverOptions.Cancel
    '/// Close 'Solver' dialog by 'Close'
    printlog " Close 'Solver' dialog by 'Close'"
    Kontext "Solver"
    Solver.Close
    '/// Close document
    printlog " Close document"
    Call hCloseDocument

endcase

'-----------------------------------------------------------

