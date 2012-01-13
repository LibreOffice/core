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
