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
'* owner : oliver.craemer@oracle.com
'*
'* short description : Chart resource test - Menu VIEW
'*
'************************************************************************
'*
' #1 tViewToolbars
' #1 tViewToolbarsCustomize
' #1 tViewFullScreen
'*
'\************************************************************************

testcase tViewToolbars
    '///<u><b>Check invocation of View::Toolbars::*</b></u>
    '/// Load simple chart document and save it in user's work dorectory
    if fLoadVerySimpleChartAndSaveLocal() > 0 then
        warnlog "Loading test document seems to have failed -> Check this out!"
        goto endsub
    endif
    '/// Select chart using the navigator
    call fSelectFirstOLE
    '/// Edit / Object / Edit
    EditObjectEdit
    '/// Convert chart to 3D
    if NOT fConvertChartTo3D() then
        warnlog "Conversion of chart to 3D failed -> Quit testcase"
        Call hCloseDocument
        goto endsub
    else
        printlog "Converting chart into 3D was successful."
    endif
    Kontext "Standardbar"    
    if Standardbar.Exists then
        '/// View / Toolbars / Standard
        ViewToolbarsStandard
        Kontext "Standardbar"
        '/// Verifying that the toolbar 'Standard' is disabled         
        if Standardbar.Exists then
            QAErrorLog "#i72537 - Disabling 'Standard' toolbar does not work."
            Call hCloseDocument
            goto endsub
        else
            '/// Enabling the toolbar again 
            ViewToolbarsStandard
        end if
    else
        warnlog "'Standard' toolbar was not visible by default as expected!"
    end if             
    Kontext "Toolbar"    
    if Toolbar.Exists then
        '/// View / Toolbars / Formatting
        ViewToolbarsTools
        Kontext "Toolbar"
        '/// Verifying that the toolbar 'Formatting' is disabled         
        if Toolbar.Exists then
            QAErrorLog "#i72537 - Disabling 'Formatting' toolbar does not work."
            Call hCloseDocument
            goto endsub            
        else
            '/// Enabling the toolbar again 
            ViewToolbarsTools
        end if
    else
        warnlog "'Standard' toolbar was not visible by default as expected!"
    end if
   '/// Close document
   Call hCloseDocument
endcase

'-------------------------------------------------------------

testcase tViewToolbarsCustomize
'///<u><b>Check Dialog after invocation of View::Toolbars::Customize</b></u>

'///<ul>
   '///+<li>Load simple chart document</li>
   if fLoadVerySimpleChartAndSaveLocal() > 0 then
       warnlog "Loading test document seems to have failed -> Check this out!"
       goto endsub
   endif
   '///+<li>Select chart using navigator</li>
   call fSelectFirstOLE
   '///+<li>Invoke Edit::Object::Edit to enter Inplace Mode</li>
   EditObjectEdit
   '///+<li>Invoke View::Toolbars::Customize</li>
   ViewToolbarsCustomize
   '///+<li>Change to Menus tab-page</li>
   Kontext
   Active.SetPage TabCustomizeMenu
   '///+<li>Check existence of dialog</li>
   Call DialogTest ( TabCustomizeMenu )
   '///+<li>Change to Keyboard tab-page</li>
   Kontext
   Active.SetPage TabTastatur
   '///+<li>Check existence of dialog</li>
   Call DialogTest ( TabTastatur )
   '///+<li>Change to Toolbars tab-page</li>
   Kontext
   Active.SetPage TabCustomizeToolbars
   '///+<li>Check existence of dialog</li>
   Call DialogTest ( TabCustomizeToolbars )
   '///+<li>Change to Events tab-page</li>
   Kontext
   Active.SetPage TabCustomizeEvents
   '///+<li>Check existence of dialog</li>
   Call DialogTest ( TabCustomizeEvents )
   '///+<li>Quit dialog</li>
   TabCustomizeEvents.cancel
   '///+<li>Close document</li>
   Kontext "DocumentCalc"
   DocumentCalc.TypeKeys "<Escape>"
   Call hCloseDocument
'///</ul>

endcase
'-------------------------------------------------------------
testcase tViewFullScreen
'///<u><b>Check invocation of View::Fullscren</b></u>

'///<ul>
   '///+<li>Load simple chart document</li>
   if fLoadVerySimpleChartAndSaveLocal() > 0 then
       warnlog "Loading test document seems to have failed -> Check this out!"
       goto endsub
   endif
   '///+<li>Select chart using navigator</li>
   call fSelectFirstOLE
   '///+<li>Invoke Edit::Object::Edit to enter Inplace Mode</li>
   EditObjectEdit
   '///+<li>Convert chart to 3D (this probably makes the test more risky)</li>
   if NOT fConvertChartTo3D() then
       warnlog "Conversion of chart to 3D failed -> Quit testcase"
       Call hCloseDocument
       goto endsub
   endif
   '///+<li>Invoke View::Fullscren</li>
   ViewFullScreen
   '///+<li>Quit fullscreen using CTRL+SHIFT+J</li>
   Kontext "DocumentCalc"
   DocumentCalc.TypeKeys "<Mod1 Shift J>"
   sleep (2)
   '///+<li>Enter fullscreen using CTRL+SHIFT+J</li>
   DocumentCalc.TypeKeys "<Mod1 Shift J>"
   '///+<li>Quit fullscreen using View::Fullscren</li>
   ViewFullScreen
   '///<li>Close document</li>
   Kontext "DocumentCalc"
   DocumentCalc.TypeKeys "<Escape>"
   Call hCloseDocument
'///</ul>

endcase
'
'-----------------------------------------------------------
'
