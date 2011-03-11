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
'* Owner : thorsten.bosbach@oracle.com
'*
'* short description :
'*
'\*****************************************************************

sub M_005_
   printlog Chr(13) +  "--  Tools Menu  m_005_ --"

   Call tToolsCatalog
   Call tToolsImportFormula
   Call tiToolsMacro
   Call tToolsExtensionManager
   
   Call tToolsCustomize 'global one ! TBO
   Call tToolsOptionstest ' global one ! TZ
end sub

testcase tToolsCatalog
   '/// open application ///'
   Call hNewDocument
   '/// Tools->Catalog ///'
   ToolsSymbolsCatalog
      Kontext "SymboleMath"
      Call Dialogtest (SymboleMath)
      printlog "count of 'Symbol set' :" + Symbolset.GetItemCount
      '/// click button 'Edit...' ///'
      Bearbeiten.Click
         Kontext "EditSymbols"
         Call DialogTest (EditSymbols)
         OldSymbol.GetItemCount
         OldSymbolSet.GetItemCount
         Symbol.GetItemCount
         SymbolSet.GetItemCount
         Subset.GetItemCount
         Typeface.GetItemCount
         '/// select last item in listbox 'Font' ///'
         Font.Select (Font.GetItemCount)
         sleep 1
         '/// Click Button 'Modify' ///'
         Modify.Click
         sleep 1
      '/// cancel dialog 'Edit Symbols' ///'
      EditSymbols.Cancel
      ' since cws tl82 - dev300m88 we need to go into dialog again to get the others buttons
      Kontext "SymboleMath"
      printlog "count of 'Symbol set' :" + Symbolset.GetItemCount
      '/// click button 'Edit...' ///'
      Bearbeiten.Click
         Kontext "EditSymbols"
         '/// select last item in listbox 'Font' ///'
         Font.Select (Font.GetItemCount)
         sleep 1
         '/// Click Button 'Delete' ///'
         Delete.Click
         sleep 1
         '/// Click Button 'Add' ///'
         Add.Click
         sleep 1
      '/// cancel dialog 'Edit Symbols' ///'
      EditSymbols.Cancel
      Kontext "SymboleMath"
      '/// click button 'Insert' ///'
      Uebernehmen.Click
      Sleep 2
   '/// close dialog 'Symbols' ///'
   SymboleMath.close
   '/// close application ///'
   Call hCloseDocument
endcase

testcase tToolsImportFormula
   '/// open application ///'
   Call hNewDocument
   Sleep 3
   '/// Tools->Import->Formula ///'
   ToolsImportFormula
   Kontext "OeffnenDlg"
      Kontext "OeffnenDlg"
      UebergeordneterOrdner.Click
      Standard.Click
      Call DialogTest ( OeffnenDlg )
   '/// cancel dialog 'Insert' ///'
   OeffnenDlg.Cancel
   Sleep 2
   '/// close application ///'
   Call hCloseDocument
endcase

testcase tiToolsMacro
'/// open application ///'
   Call hNewDocument
   sleep 2
   '/// try to call Tools->Macros...->Record Macro => this has to fail, it is only aloowed from writer and Calc ///'
   try
      ToolsMacroRecordMacro
      Warnlog "I can record macros in this application :-("
   catch
      Printlog "i can't record macros in this application :-)"
   endcatch
'/// Tools->Macro ///'
   ToolsMacro
   Kontext "Makro"
   Call DialogTest ( Makro )
'/// click button 'organizer...' ///'
   Verwalten.Click

   Kontext
'/// switch to tabpage 'Modules' ///'
   Messagebox.SetPage TabModule
   Kontext "TabModule"
   Call DialogTest ( TabModule )

   Kontext
'/// switch to tabpage 'Libraries' ///'
   Messagebox.SetPage TabBibliotheken
   Kontext "TabBibliotheken"
   Call DialogTest ( TabBibliotheken )
'/// click lbutton 'append' ///'
      Hinzufuegen.Click
      Kontext "Messagebox"
      if Messagebox.Exists (5) then
     if Messagebox.GetRT = 304 then
            Warnlog Messagebox.Gettext
            Messagebox.Ok
     endif
      end if
      Kontext "OeffnenDlg"
'/// cancel dialog 'append libraries' ///'
      OeffnenDlg.Cancel
      Kontext "TabBibliotheken"
'/// click button 'new' ///'
      Neu.Click
      kontext "NeueBibliothek"
      sleep 1 'Bibliotheksname
'/// cancel dialog 'new library' ///'
      NeueBibliothek.cancel
   Kontext "TabBibliotheken"
'/// close dialog 'macro organizer' ///'
   TabBibliotheken.Close

   Kontext "Makro"
'/// close dialog 'macro' ///'
   Makro.Cancel
'/// close application ///'
   Call hCloseDocument
endcase

testcase tToolsExtensionManager
    dim sPath as string
    dim sTemp as string
    dim sTemp2 as string
    dim x as integer

    if ( lcase( gPlatform ) <> "osx" ) then
    sPath = convertPath(gTestToolPath+"math/required/input/")
    '/// open application ///'
    Call hNewDocument
    sleep 2
    '/// call Tools - Package/Extension manager ///'
    ToolsPackageManager
        kontext "PackageManager"
        If PackageManager.exists(10) then
            DialogTest(PackageManager)
            '/// going to top of package list and explode every line, until the button 'Add' is enabled ///'
            '/// should be 'My Packages' ///'
            BrowsePackages.typeKeys("<home>")
            if (Not Add.isEnabled) then
                warnlog "Couldn't find a way to enable the Add button - aborting"
                goto endsub
            endif
            '/// click button 'Add' ///'
            Add.click
                '/// Add Package(s)... dialg comes up ///'
                Kontext "OeffnenDlg"
                if OeffnenDlg.exists(5) then
                    '/// put filename into field ///'
                    Dateiname.setText (sPath + "unknown-dependency.oxt")
                    '/// press button 'Open' ///'
                    Oeffnen.click
                else
                    warnlog "Add Package(s)... dialog didn't came up"
                endif
                
            'Asking for whom to install; All: YES, Me: NO, nobody: Cancel
             kontext
             if active.exists(5) then
                sTemp = active.getText
                try
                active.yes
                catch
                'printlog sTemp
                endcatch
             endif

                '/// Progress dialog comes up ///'
                Kontext "AddingPackages"
                if AddingPackages.exists(1) then
                    printlog " - AddingPackages dialog exists"
                    DialogTest(AddingPackages)
                endif
                
                '/// If any error happens, an active comes up, telling what is wrong ///'
                Kontext
                if active.exists(5) then
                    sTemp = active.getText
                        printlog sTemp
                    active.ok
                endif
                
                '/// If there are any unsatisfied dependencies, they are shown now ///'
                Kontext "UnsatisfiedDependencies"
                if UnsatisfiedDependencies.exists(10) then
                        printlog " - UnsatisfiedDependencies dialog exists"
                    DialogTest(UnsatisfiedDependencies)
                    '/// close dialog if available, extension will not be installed ///'
                    UnsatisfiedDependencies.ok
                endif

                Kontext "AddingPackages"
                x=0
                while AddingPackages.exists(1) AND (x<6)
                    printlog "AddingPackages..."
                    sleep 5
                    x=x+1
                wend
                
             kontext "PackageManager"
             Add.click
                '/// Add Package(s)... dialg comes up ///'
                Kontext "OeffnenDlg"
                if OeffnenDlg.exists(5) then
                    '/// put filename into field ///'
                    Dateiname.setText (sPath + "ShortLicense.oxt")
                    '/// press button 'Open' ///'
                    Oeffnen.click
                else
                    warnlog "Add Package(s)... dialog didn't came up"
                endif
                
            'Asking for whom to install; All: YES, Me: NO, nobody: Cancel
             kontext
             if active.exists(5) then
                sTemp = active.getText
                try
                active.yes
                catch
                'printlog sTemp
                endcatch
             endif

                '/// Progress dialog comes up ///'
                Kontext "AddingPackages"
                if AddingPackages.exists(1) then
                    printlog " - AddingPackages dialog exists"
                endif
                
                '/// If any error happens, an active comes up, telling what is wrong ///'
                Kontext
                if active.exists(5) then
                    sTemp = active.getText
                        printlog sTemp
                    active.ok
                endif
                
                '/// If there is a license, it is shown now ///'
                Kontext "ExtensionSoftwareLicenseAgreement"
                if ExtensionSoftwareLicenseAgreement.exists(10) then
                    DialogTest (ExtensionSoftwareLicenseAgreement)
                        printlog " - ExtensionSoftwareLicenseAgreement dialog exists"
                            ExtensionSoftwareLicenseAgreement.CANCEL
                    kontext
                    if active.exists(5) then
                        '#i73307# 
                    	warnlog "Alzheimer feature extension licensing '" + active.getText + "'"
                    	active.ok
                    endif
                endif

                Kontext "AddingPackages"
                x=0
                while AddingPackages.exists(1) AND (x<6)
                    printlog "AddingPackages..."
                    sleep 5
                    x=x+1
                wend
                
            '/// Close extension manager ///'
            kontext "PackageManager"
            PackageManager.OK
        else
            warnlog "Couldn't call Tools -> Package manager - aborting test"
        endif
   Call hCloseDocument
   else
       qaErrorLog "Do not run on macosx - not reproduceable error"
   endif
endcase
