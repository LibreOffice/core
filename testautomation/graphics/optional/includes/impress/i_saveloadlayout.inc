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
'* Owner : wolfram.garten@sun.com
'*
'* short description: Save & Load testing of Layout -templates.
'*
'**************************************************************************************

testcase tSaveLoadLayoutEmpty
    Dim NewFileDir as String
    NewFileDir = ConvertPath (gOfficePath + "user\work\LayoutTest\")

    printlog "Create New folder in the Work-directory"
    printlog "    Will try to create the directory: " + NewFileDir
    app.mkdir NewFileDir

    printlog "Create a new document, add an empty Layout, Save the document in all available Formats, and open the saved files."

    printlog "Make new Presentation"
    gApplication = "IMPRESS"
    Call hNewDocument

    printlog "Choose and Insert an empty Layout."
    printlog "   Choose and Insert an empty Layout."
    FormatModifyPage
    sleep (1)
    kontext "Tasks"
    LayoutsPreview.TypeKeys "<HOME>"
    kontext "Pagelayout_UndoDeleteWarning"
    if Pagelayout_UndoDeleteWarning.exists then
        Pagelayout_UndoDeleteWarning.ok
    end if
    kontext "Tasks"
    printlog "Press Enter to use the layout on the current slide"
    LayoutsPreview.TypeKeys "<RETURN>"
    sleep (5)

    printlog "Save the document in different formats..."
    printlog "Close the file."
    printlog "Load the different files."
    call fSaveLoadAllFormats (NewFileDir)   'Runs the Function below. 

    printlog "Delete the different files."
    printlog "   Will try to delete the directory: " + NewFileDir
    app.rmDir NewFileDir

    printlog "tSaveLoadLayoutEmpty ended."
    printlog "End the test"
endcase 'tSaveLoadLayoutEmpty

'****************************************************************************************************

testcase tSaveLoadLayoutText
    printlog "Testing layout with text."
    Dim NewFileDir as String
    NewFileDir = ConvertPath (gOfficePath + "user\work\LayoutTest\")

    printlog "Create New folder in the Work-directory"
    printlog "    Will try to create the directory: " + NewFileDir
    app.mkdir NewFileDir

    printlog "Create a new document, add a Layout with a Textfield, Save the document in all available Formats, and open the saved files."

    printlog "Make new Presentation"
    gApplication = "IMPRESS"
    Call hNewDocument

    printlog "Choose and Insert an Layout with a Text-field."
    FormatModifyPage
    sleep (1)
    kontext "Tasks"
    LayoutsPreview.TypeKeys "<HOME>"
    kontext "Pagelayout_UndoDeleteWarning"
    if Pagelayout_UndoDeleteWarning.exists then
        Pagelayout_UndoDeleteWarning.ok
    end if
    kontext "Tasks"
    printlog "select the Text placeholder and activate it with [Return]"
    LayoutsPreview.TypeKeys ("<RIGHT>",5)
    sleep (1)
    LayoutsPreview.TypeKeys ("<Return>")
    sleep (1)
    kontext "DocumentImpress"
    DocumentImpress.TypeKeys ("<TAB>")
    DocumentImpress.TypeKeys ("<Return>")
    DocumentImpress.TypeKeys ("<ESCAPE>")
    sleep (5)

    printlog "Save the document in all available formats..."
    printlog "Close the file."
    printlog "Load the different files."
    call fSaveLoadAllFormats (NewFileDir)   'Runs the Function below.

    printlog "Delete the different files."
    printlog "Will try to delete the directory: " + NewFileDir
    app.rmDir NewFileDir 

    printlog "tSaveLoadLayoutText ended."
endcase 'tSaveLoadLayoutText

'****************************************************************************************************

testcase tSaveLoadLayoutContent
    Dim NewFileDir as String
    NewFileDir = ConvertPath (gOfficePath + "user\work\LayoutTest\")

    printlog "Create New folder in the Work-directory"
    printlog "   Will try to create the directory: " + NewFileDir
    app.mkdir NewFileDir

    printlog "Create a new document, add a Layout with a Content-field, Save the document in all available Formats, and open the saved files."

    printlog "Make new Presentation"
    gApplication = "IMPRESS"
    Call hNewDocument

    printlog "Choose and Insert an Layout with a Content-field."
    FormatModifyPage
    sleep (1)
    kontext "Tasks"
    LayoutsPreview.TypeKeys "<HOME>"
    kontext "Pagelayout_UndoDeleteWarning"
    if Pagelayout_UndoDeleteWarning.exists then
       Pagelayout_UndoDeleteWarning.ok
    end if
    kontext "Tasks"
    printlog "select the Picture placeholder and activate it with [Return]"
    LayoutsPreview.TypeKeys ("<RIGHT>",2)
    sleep (1)
    LayoutsPreview.TypeKeys ("<Return>")
    sleep (1)
    kontext "DocumentImpress"
    DocumentImpress.TypeKeys ("<TAB><TAB>")
    DocumentImpress.TypeKeys ("<Return>")
    DocumentImpress.TypeKeys ("<ESCAPE>")
    sleep (5)

    printlog "Save the document in all available formats."
    printlog "Close the file."
    printlog "Load the different files."
    call fSaveLoadAllFormats (NewFileDir)   'Runs the Function below.

    printlog "Delete the different files."
    printlog "   Will try to delete the directory: " + NewFileDir
    app.rmDir NewFileDir 'ConvertPath (gOfficePath + "user\work\LayoutTest\")

    printlog "tSaveLoadLayoutPicture ended."
endcase 'tSaveLoadLayoutPicture

'****************************************************************************************************
