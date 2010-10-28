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
'* short description : CROSS-APPLICATIONS CLIPBOARD TEST (Writer)
'*
'\***********************************************************************

sub w_220_

    Call HTMLToHTMLText
    Call HTMLToHTMLField
    Call HTMLToHTMLTable
    Call HTMLToHTMLGraphicLinked
    Call HTMLToHTMLControl
    Call HTMLToHTMLFloatingFrame

end sub

' ---------------------------------------------------------------------------------

testcase HTMLToHTMLText

'    if lcase(gPlatform) = "x86" and Val(mid$(gMinor, 2, Len(gMinor))) <= 129  then
'        QAErrorlog "#125780#Stacktrace ID: 251397"
'        goto endsub
'    end if    
    gApplication = "HTML"
    printlog "  Open File '..\\writer\\optional\\input\\clipboard\\html.html' "
    printlog "  Jump to beginning of document "
    printlog "  Select first paragraph "
    printlog "  Copy selected text "
    hFileOpenLocally( gtesttoolpath & "writer\optional\input\clipboard\html.html")
    
    printlog "  Jump to beginning of document "
    Call wTypeKeys "<Mod1 Home>"
    Call wTypeKeys "<Shift Down><Shift End>"
    EditCopy
    
    gApplication = "HTML"
    printlog "  Open new document "
    Call hNewDocument
    printlog "  Edit / Paste Special "
    Sleep 2
    try
        EditPasteSpecialWriter
    catch
        Warnlog "Unable to execute 'Edit / Paste Special' ! Test failed!"
        Call hCloseDocument ' Close Source-File
        Call hCloseDocument ' Close Target-File
        goto endsub
    endcatch
    Wait 500
    printlog "  Paste 'Text' in all available clipboard formats "
    Kontext "InhaltEinfuegen"
    if InhaltEinfuegen.Exists then
        Call wPasteAvailableClipboardFormats(Auswahl.GetItemCount,"text")
        printlog "  Write Clipboard format under pasted content "
        Call hFileSaveAsWithFilterKill (gOfficepath + "user\work\html_to_html_text.html", "HTML")
        printlog "  Save document as ..\user\work\html_to_html_text.html"
        Wait 500
        Call hCloseDocument
        printlog "  Close saved document "
        wait 500
        hFileOpen( gOfficepath + "user\work\html_to_html_text.html",false)
        printlog "  Reopen saved document "
        Wait 500
        Call hCloseDocument
        printlog "  Close saved document "
    else
        Warnlog "Dialog 'Paste Special' is not up!"
        Call hCloseDocument
        goto endsub
    end if
    printlog "  Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

' ----------------------------------------------------------------------------------------------

testcase HTMLToHTMLField
    gApplication = "HTML"
    printlog "  Open File '..\\writer\\optional\\input\\clipboard\\html.html' "
    printlog "  Jump to beginning of document "
    printlog "  Select paragraph with 'Date Field' "
    printlog "  Copy selected text "
    hFileOpenLocally( gtesttoolpath & "writer\optional\input\clipboard\html.html")
    
    printlog "  Jump to beginning of document "
    Call wTypeKeys "<Mod1 Home>"
    Call wTypeKeys "<Down>",2
    Call wTypeKeys "<Home><Shift End>"
    EditCopy
    
    gApplication = "HTML"
    printlog "  Open new document "
    Call hNewDocument
    printlog "  Edit / Paste Special "
    Sleep 2
    try
        EditPasteSpecialWriter
    catch
        Warnlog "Unable to execute 'Edit / Paste Special' ! Test failed!"
        Call hCloseDocument ' Close Source-File
        Call hCloseDocument ' Close Target-File
        goto endsub
    endcatch
    Wait 500
    printlog "  Paste 'Field' in all available clipboard formats "
    Kontext "InhaltEinfuegen"
    if InhaltEinfuegen.Exists then
        Call wPasteAvailableClipboardFormats(Auswahl.GetItemCount,"field")
        printlog "  Write Clipboard format under pasted content "
        Call hFileSaveAsWithFilterKill (gOfficepath + "user\work\html_to_html_field.html", "HTML")
        printlog "  Save document as ..\user\work\html_to_html_field.html"
        Wait 500
        Call hCloseDocument
        printlog "  Close saved document "
        wait 500
        hFileOpen( gOfficepath + "user\work\html_to_html_field.html" )
        printlog "  Reopen saved document "
        Wait 500
        Call hCloseDocument
        printlog "  Close saved document "
    else
        Warnlog "Dialog 'Paste Special' is not up!"
        Call hCloseDocument
        goto endsub
    end if
    printlog "  Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

' ----------------------------------------------------------------------------------------------

testcase HTMLToHTMLTable
    gApplication = "HTML"
    printlog "  Open File '..\\writer\\optional\\input\\clipboard\\html.html' "
    printlog "  Jump to beginning of document "
    printlog "  Select 'Table' "
    printlog "  Copy selected table "
    hFileOpenLocally( gtesttoolpath & "writer\optional\input\clipboard\html.html")
    
    printlog "  Jump to beginning of document "
    Call wTypeKeys "<Mod1 Home>"
    Call wTypeKeys "<Down>",5
    Call wTypeKeys "<Mod1 A>",2
    EditCopy
    
    gApplication = "HTML"
    printlog "  Open new document "
    Call hNewDocument
    printlog "  Edit / Paste Special "
    Sleep 2
    try
        EditPasteSpecialWriter
    catch
        Warnlog "Unable to execute 'Edit / Paste Special' ! Test failed!"
        Call hCloseDocument ' Close Source-File
        Call hCloseDocument ' Close Target-File
        goto endsub
    endcatch
    Wait 500
    printlog "  Paste 'Table' in all available clipboard formats "
    Kontext "InhaltEinfuegen"
    if InhaltEinfuegen.Exists then
        Call wPasteAvailableClipboardFormats(Auswahl.GetItemCount,"table")
        printlog "  Write Clipboard format under pasted content "
        Call hFileSaveAsWithFilterKill (gOfficepath + "user\work\html_to_html_table.html", "HTML")
        printlog "  Save document as ..\user\work\html_to_html_table.html"
        Wait 500
        Call hCloseDocument
        printlog "  Close saved document "
        wait 500
        hFileOpen( gOfficepath + "user\work\html_to_html_table.html",false)
        printlog "  Reopen saved document "
        Wait 500
        Call hCloseDocument
        printlog "  Close saved document "
    else
        Warnlog "Dialog 'Paste Special' is not up!"
        Call hCloseDocument
        goto endsub
    end if
    printlog "  Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

' ----------------------------------------------------------------------------------------------

testcase HTMLToHTMLGraphicLinked
    gApplication = "HTML"
    printlog "  Open File '..\\writer\\optional\\input\\clipboard\\html.html' "
    printlog "  Jump to beginning of document "
    printlog "  Select 'Linked Graphic' "
    printlog "  Copy selected Linked Graphic "
    hFileOpenLocally( gtesttoolpath & "writer\optional\input\clipboard\html.html")
    
    printlog "  Jump to beginning of document "
    Call wTypeKeys "<Mod1 Home>"
    Call wTypeKeys ( "<Shift F4>" )
    EditCopy

    gApplication = "HTML"
    printlog "  Open new document "
    Call hNewDocument
    printlog "  Edit / Paste Special "
    Sleep 2
    try
       EditPasteSpecialWriter
    catch
       Warnlog "Unable to execute 'Edit / Paste Special' ! Test failed!"
       Call hCloseDocument ' Close Source-File
       Call hCloseDocument ' Close Target-File
       goto endsub
    endcatch
    Wait 500
    printlog "  Paste 'Linked Graphic' in all available clipboard formats "
    Kontext "InhaltEinfuegen"
    if InhaltEinfuegen.Exists then
        Call wPasteAvailableClipboardFormats(Auswahl.GetItemCount,"graphicL")
        printlog "  Write Clipboard format under pasted content "
        Call hFileSaveAsWithFilterKill (gOfficepath + "user\work\html_to_html_graphic1.html", "HTML")
        printlog "  Save document as ..\user\work\html_to_html_graphic1.html"
        Wait 500
        Call hCloseDocument
        printlog "  Close saved document "
        sleep (2)
        hFileOpen( gOfficepath + "user\work\html_to_html_graphic1.html",false)
        printlog "  Reopen saved document "
        Wait 500
        Call hCloseDocument
        printlog "  Close saved document "
    else
        Warnlog "Dialog 'Paste Special' is not up!"
        Call hCloseDocument
        goto endsub
    end if
    printlog "  Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

' ----------------------------------------------------------------------------------------------

testcase HTMLToHTMLControl
    gApplication = "HTML"
    printlog "  Open File '..\\writer\\optional\\input\\clipboard\\html.html' "
    printlog "  Jump to beginning of document "
    printlog "  Select 'Control' "
    printlog "  Copy selected Control "
    hFileOpenLocally( gtesttoolpath & "writer\optional\input\clipboard\html.html")
    
    printlog "  Jump to beginning of document "
    Call wTypeKeys "<Mod1 Home>"
    Call wTypeKeys ( "<Shift F4>" )
    Call wTypeKeys "<Tab>"
    EditCopy

    gApplication = "HTML"
    printlog "  Open new document "
    Call hNewDocument
    printlog "  Edit / Paste Special "
    Sleep 2
    try
       EditPasteSpecialWriter
    catch
       Warnlog "Unable to execute 'Edit / Paste Special' ! Test failed!"
       Call hCloseDocument ' Close Source-File
       Call hCloseDocument ' Close Target-File
       goto endsub
    endcatch
    Wait 500
    printlog "  Paste 'Control' in all available clipboard formats "
    Kontext "InhaltEinfuegen"
    if InhaltEinfuegen.Exists then
        Call wPasteAvailableClipboardFormats(Auswahl.GetItemCount,"control")
        printlog "  Write Clipboard format under pasted content "
        Call hFileSaveAsWithFilterKill (gOfficepath + "user\work\html_to_html_control.html", "HTML")
        printlog "  Save document as ..\user\work\html_to_html_control.html"
        Wait 500
        Call hCloseDocument
        printlog "  Close saved document "
        wait 500
        hFileOpen( gOfficepath + "user\work\html_to_html_control.html",false)
        printlog "  Reopen saved document "
        Wait 500
        Call hCloseDocument
        printlog "  Close saved document "
    else
        Warnlog "Dialog 'Paste Special' is not up!"
        Call hCloseDocument
        goto endsub
    end if
    printlog "  Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase

' ----------------------------------------------------------------------------------------------

testcase HTMLToHTMLFloatingFrame
    gApplication = "HTML"
    printlog "  Open File '..\\writer\\optional\\input\\clipboard\\html.html' "
    printlog "  Jump to beginning of document "
    printlog "  Select 'Floating Frame' "
    printlog "  Copy selected Floating Frame "
    hFileOpenLocally( gtesttoolpath & "writer\optional\input\clipboard\html.html")
    
    printlog "  Jump to beginning of document "
    Call wTypeKeys "<Mod1 Home>"
    Call wTypeKeys ( "<Shift F4>" )
    Call wTypeKeys "<Tab>",2
    try
        EditCopy
    catch
        Warnlog "Edit / Copy is disabled ! Maybe object not selected!"
        Call hCloseDocument
        goto endsub
    endcatch
    gApplication = "HTML"
    printlog "  Open new document "
    Call hNewDocument
    printlog "  Edit / Paste Special "
    Sleep 2
    try
        EditPasteSpecialWriter
    catch
        Warnlog "Unable to execute 'Edit / Paste Special' ! Test failed!"
        Call hCloseDocument ' Close Source-File
        Call hCloseDocument ' Close Target-File
        goto endsub
    endcatch
    Wait 500
    printlog "  Paste 'Floating Frame' in all available clipboard formats "
    Kontext "InhaltEinfuegen"
    if InhaltEinfuegen.Exists then
        Call wPasteAvailableClipboardFormats(Auswahl.GetItemCount,"control")
        printlog "  Write Clipboard format under pasted content "
        Call hFileSaveAsWithFilterKill (gOfficepath + "user\work\html_to_html_float.html", "HTML")
        printlog "  Save document as ..\user\work\html_to_html_float.html"
        Wait 500
        Call hCloseDocument
        printlog "  Close saved document "
        wait 500
        hFileOpen( gOfficepath + "user\work\html_to_html_float.html",false)
        printlog "  Reopen saved document "
        Wait 500
        Call hCloseDocument
        printlog "  Close saved document "
    else
        Warnlog "Dialog 'Paste Special' is not up!"
        Call hCloseDocument
        goto endsub
    end if
    printlog "  Close active document "
    Do Until GetDocumentCount = 0
        Call hCloseDocument
    Loop
endcase
