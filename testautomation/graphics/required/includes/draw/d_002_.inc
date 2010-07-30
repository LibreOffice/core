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
'* Owner : wolfram.garten@oracle.com
'*
'* short description :
'*
'\*****************************************************************

sub d_002

    call tdEditCrossFading()
    call tdEditLayer()

end sub

'*******************************************************************************

testcase tdEditCrossFading

    printlog " open application "
    Call hNewDocument
    printlog " create 2 rectangles "
    gMouseClick 50,50
    Call hRechteckErstellen ( 10, 10, 20, 40 )
    Call hRechteckErstellen ( 30, 30, 50, 60 )
    printlog " Edit-YSelect All "
    EditSelectAll
    try
        printlog " Edit->Cross-fading "
        EditCrossFading
    catch
        warnlog "EditCrossFading not accessible :-("
    endcatch
    
    Kontext "Ueberblenden"
    Call DialogTest ( Ueberblenden )
    printlog " Change : 'Increments'; 1 more, 1 less "
    Schritte.More
    Schritte.Less
    printlog " Change: Cross-fading attributes; uncheck, check "
    Attributierung.uncheck
    Attributierung.check
    printlog " Change: same orientation; uncheck, check "
    GleicheOrientierung.Uncheck
    GleicheOrientierung.Check
    printlog " cancel dialog 'Cross-fading'; uncheck, check "
    Ueberblenden.Cancel
    printlog " close application "
    Call hCloseDocument
    
endcase 'tdEditCrossFading
'------------------------------------------------------------------------------
testcase tdEditLayer

    printlog " open application "
    Call hNewDocument
    printlog " View->Layer "
    ViewLayer
    printlog " Edit->Layer->Insert "
    InsertLayer
    Kontext "EbeneEinfuegenDlg"
    Call DialogTest ( EbeneEinfuegenDlg )
    printlog " Change: Set another name for the layer "
    EbenenName.SetText "SomeThing"
    printlog " Change: Visible; uncheck, check "
    Sichtbar.UnCheck
    Sichtbar.Check
    printlog " Change: Printable; uncheck, check "
    Druckbar.UnCheck
    Druckbar.Check
    printlog " Change: Locked; check, uncheck "
    Gesperrt.Check
    Gesperrt.UnCheck
    EbeneEinfuegenDlg.OK
    printlog " (Edit->Layer->Modify is tested in format-menu-test) "
    printlog " Edit->Layer->Rename "
    EditLayerRename
    kontext "DocumentDrawImpress"
    LayerTabBar.TypeKeys "Apply!!<Return>" , true
    printlog " Edit->Layer->Delete "
    EditDeleteLayer    
    printlog " Messagebox: really delete? YES "
    Kontext "Messagebox"
    Messagebox.Yes
    sleep (2)
    printlog " View->Layer "
    ViewLayer
    printlog " close application "
    Call hCloseDocument

endcase 'tdEditLayer
'------------------------------------------------------------------------------
