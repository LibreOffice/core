'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: i_saveloadlayout.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 10:42:42 $
'*
'* This file is part of OpenOffice.org.
'*
'* OpenOffice.org is free software: you can redistribute it and/or modify
'* it under the terms of the GNU Lesser General Public License version 3
'* only, as published by the Free Software Foundation.
'*
'* OpenOffice.org is distributed in the hope that it will be useful,
'* but WITHOUT ANY WARRANTY; without even the implied warranty of
'* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
'* GNU Lesser General Public License version 3 for more details
'* (a copy is included in the LICENSE file that accompanied this code).
'*
'* You should have received a copy of the GNU Lesser General Public License
'* version 3 along with OpenOffice.org.  If not, see
'* <http://www.openoffice.org/license.html>
'* for a copy of the LGPLv3 License.
'*
'/************************************************************************
'*
'* Owner : wolfram.garten@sun.com
'*
'* short description: Save & Load testing of Layout -templates.
'*
'**************************************************************************************
' #1 tSaveLoadLayoutEmpty
' #1 tSaveLoadLayoutOLE
' #1 tSaveLoadLayoutText
' #1 tSaveLoadLayoutOutline
' #1 tSaveLoadLayoutPicture
' #1 tSaveLoadLayoutChart
' #1 tSaveLoadLayoutSpreadsheet
'\*************************************************************************************

testcase tSaveLoadLayoutEmpty
    goto endsub
    Dim NewFileDir as String
    NewFileDir = ConvertPath (gOfficePath + "user\work\LayoutTest\")

    '/// Create New folder in the Work-directory ///'
    printlog "    Will try to create the directory: " + NewFileDir
    app.mkdir NewFileDir

    '/// Create a new document, add an empty Layout, Save the document in all available Formats, and open the saved files. ///'

    '/// Make new Presentation ///'
    gApplication = "IMPRESS"
    Call hNewDocument

    '/// Choose and Insert an empty Layout. ///'
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
    '/// Press "Enter" to use the layout on the current slide ///'
    LayoutsPreview.TypeKeys "<RETURN>"
    sleep (5)

    '/// Save the document in different formats... ///'
    '/// Close the file. ///'
    '/// Load the different files. ///'
    call fSaveLoadAllFormats (NewFileDir)   'Runs the Function below. 

    '/// Delete the different files. ///'
    printlog "   Will try to delete the directory: " + NewFileDir
    app.rmDir NewFileDir

    printlog "tSaveLoadLayoutEmpty ended."
    '/// End the test ///'
endcase 'tSaveLoadLayoutEmpty

'****************************************************************************************************

testcase tSaveLoadLayoutOLE
    Dim NewFileDir as String
    NewFileDir = ConvertPath (gOfficePath + "user\work\LayoutTest\")

    '/// Create New folder in the Work-directory ///'
    printlog "   Will try to create the directory: " + NewFileDir
    app.mkdir NewFileDir

    '/// Create a new document, add a Layout with an OLE-field, Save the document in all available Formats, and open the saved files. ///'

    '/// Make new Presentation ///'
    gApplication = "IMPRESS"
    Call hNewDocument

    '/// Choose and Insert an Layout with an OLE. ///'
    printlog "   Choose and Insert an Layout with an OLE."
    FormatModifyPage
    sleep (1)
    kontext "Tasks"
        LayoutsPreview.TypeKeys "<HOME>"
    kontext "Pagelayout_UndoDeleteWarning"
    if Pagelayout_UndoDeleteWarning.exists then
        Pagelayout_UndoDeleteWarning.ok
    end if
    kontext "Tasks"
        '/// select the OLE placeholder and activate it with [Return] ///'
        LayoutsPreview.TypeKeys ("<RIGHT>",13)
        sleep (1)
        LayoutsPreview.TypeKeys ("<Return>")
        sleep (1)
        kontext "DocumentImpress"
        DocumentImpress.TypeKeys ("<TAB><TAB><TAB>")
        DocumentImpress.TypeKeys ("<Return>")
        try
            sleep (2)
            kontext "OLEObjektInsert"
            sleep (1)
            OLEObjektInsert.Cancel
            Printlog "   Correctly inserted a layout with an 'Insert OLE-Object'-frame."
        catch
            warnlog "Ole wasn't selected :-("
	    CALL hCloseDocument
	    Goto Endsub
        endcatch
    sleep (5)

    '/// Save the document in all available formats... ///'
    '/// Close the file. ///'
    '/// Load the different files. ///'
    call fSaveLoadAllFormats (NewFileDir)   'Runs the Function below.

    '/// Delete the different files. ///'
    printlog "    Will try to delete the directory: " + NewFileDir
    app.rmDir NewFileDir

    printlog "tSaveLoadLayoutOLE ended."
    '/// End the test ///'
endcase 'tSaveLoadLayoutOLE

'****************************************************************************************************

testcase tSaveLoadLayoutText
    Dim NewFileDir as String
    NewFileDir = ConvertPath (gOfficePath + "user\work\LayoutTest\")

    '/// Create New folder in the Work-directory ///'
    printlog "    Will try to create the directory: " + NewFileDir
    app.mkdir NewFileDir

    '/// Create a new document, add a Layout with a Textfield, Save the document in all available Formats, and open the saved files. ///'

    '/// Make new Presentation ///'
    gApplication = "IMPRESS"
    Call hNewDocument

    '/// Choose and Insert an Layout with a Text-field. ///'
    printlog "    Choose and Insert an Layout with a Text-field."
    FormatModifyPage
    sleep (1)
    kontext "Tasks"
        LayoutsPreview.TypeKeys "<HOME>"
        kontext "Pagelayout_UndoDeleteWarning"
        if Pagelayout_UndoDeleteWarning.exists then
        Pagelayout_UndoDeleteWarning.ok
    end if
    kontext "Tasks"
        '/// select the Text placeholder and activate it with [Return] ///'
        LayoutsPreview.TypeKeys ("<RIGHT>")
        sleep (1)
        LayoutsPreview.TypeKeys ("<Return>")
        sleep (1)
       kontext "DocumentImpress"
       DocumentImpress.TypeKeys ("<TAB><TAB><TAB>")
       DocumentImpress.TypeKeys ("<Return>")
       DocumentImpress.TypeKeys ("<ESCAPE>")
'       LayoutsPreview.TypeKeys "<RETURN>"
    sleep (5)

    '/// Save the document in all available formats... ///'
    '/// Close the file. ///'
    '/// Load the different files. ///'
    call fSaveLoadAllFormats (NewFileDir)   'Runs the Function below.

    '/// Delete the different files. ///'
    printlog "    Will try to delete the directory: " + NewFileDir
    app.rmDir NewFileDir 

    printlog "tSaveLoadLayoutText ended."
    '/// End the test ///'
endcase 'tSaveLoadLayoutText

'****************************************************************************************************

testcase tSaveLoadLayoutOutline
    Dim NewFileDir as String
    NewFileDir = ConvertPath (gOfficePath + "user\work\LayoutTest\")

    '/// Create New folder in the Work-directory ///'
    printlog "   Will try to create the directory: " + NewFileDir
    app.mkdir NewFileDir

    '/// Create a new document, add a Layout with an Outline-field, Save the document in all available Formats, and open the saved files. ///'

    '/// Make new Presentation ///'
    gApplication = "IMPRESS"
    Call hNewDocument

    '/// Choose and Insert an Layout with a Outline-field. ///'
    printlog "   Choose and Insert an Layout with a Outline-field."
    FormatModifyPage
    sleep (1)
    kontext "Tasks"
       LayoutsPreview.TypeKeys "<HOME>"
       kontext "Pagelayout_UndoDeleteWarning"
       if Pagelayout_UndoDeleteWarning.exists then
       Pagelayout_UndoDeleteWarning.ok
    end if
    kontext "Tasks"
       '/// select the Outline placeholder and activate it with [Return] ///'
       LayoutsPreview.TypeKeys ("<RIGHT>",3)
       sleep (1)
       LayoutsPreview.TypeKeys ("<Return>")
       sleep (1)
       kontext "DocumentImpress"
       DocumentImpress.TypeKeys ("<TAB><TAB><TAB>")
       DocumentImpress.TypeKeys ("<Return>")
       DocumentImpress.TypeKeys ("<ESCAPE>")
    sleep (5)

    '/// Save the document in all available formats... ///'
    '/// Close the file. ///'
    '/// Load the different files. ///'
    call fSaveLoadAllFormats (NewFileDir)   'Runs the Function below.

    '/// Delete the different files. ///'
    printlog "   Will try to delete the directory: " + NewFileDir
    app.rmDir NewFileDir 

    printlog "tSaveLoadLayoutOutline ended."
    '/// End the test ///'
endcase 'tSaveLoadLayoutOutline

'****************************************************************************************************

testcase tSaveLoadLayoutPicture
    Dim NewFileDir as String
    NewFileDir = ConvertPath (gOfficePath + "user\work\LayoutTest\")

    '/// Create New folder in the Work-directory ///'
    printlog "   Will try to create the directory: " + NewFileDir
    app.mkdir NewFileDir

    '/// Create a new document, add a Layout with a Graphic-field, Save the document in all available Formats, and open the saved files. ///'

    '/// Make new Presentation ///'
    gApplication = "IMPRESS"
    Call hNewDocument

    '/// Choose and Insert an Layout with a Graphic-field. ///'
    printlog "   Choose and Insert an Layout with a Graphic-field."
    FormatModifyPage
    sleep (1)
    kontext "Tasks"
       LayoutsPreview.TypeKeys "<HOME>"
       kontext "Pagelayout_UndoDeleteWarning"
       if Pagelayout_UndoDeleteWarning.exists then
       Pagelayout_UndoDeleteWarning.ok
    end if
    kontext "Tasks"
       '/// select the Picture placeholder and activate it with [Return] ///'
       LayoutsPreview.TypeKeys ("<RIGHT>",8)
       sleep (1)
       LayoutsPreview.TypeKeys ("<Return>")
       sleep (1)
       kontext "DocumentImpress"
       DocumentImpress.TypeKeys ("<TAB><TAB><TAB>")
       DocumentImpress.TypeKeys ("<Return>")
       DocumentImpress.TypeKeys ("<ESCAPE>")
       sleep (5)

    '/// Save the document in all available formats. ///'
    '/// Close the file. ///'
    '/// Load the different files. ///'
    call fSaveLoadAllFormats (NewFileDir)   'Runs the Function below.

    '/// Delete the different files. ///'
    printlog "   Will try to delete the directory: " + NewFileDir
    app.rmDir NewFileDir 'ConvertPath (gOfficePath + "user\work\LayoutTest\")

    printlog "tSaveLoadLayoutPicture ended."
    '/// End the test ///'
endcase 'tSaveLoadLayoutPicture

'****************************************************************************************************

testcase tSaveLoadLayoutChart
    Dim NewFileDir as String
    NewFileDir = ConvertPath (gOfficePath + "user\work\LayoutTest\")

    '/// Create New folder in the Work-directory ///'
    printlog "   Will try to create the directory: " + NewFileDir
    app.mkdir NewFileDir

    '/// Create a new document, add a Layout for a Chart, Save the document in all available formats, and open the saved files. ///'

    '/// Make new Presentation ///'
    gApplication = "IMPRESS"
    Call hNewDocument

    '/// Choose and Insert an Layout with a Chart-field. ///'
    printlog "   Choose and Insert an Layout with a Chart-field."
    FormatModifyPage
    sleep (1)
    kontext "Tasks"
       LayoutsPreview.TypeKeys "<HOME>"
       kontext "Pagelayout_UndoDeleteWarning"
       if Pagelayout_UndoDeleteWarning.exists then
       Pagelayout_UndoDeleteWarning.ok
    end if
    kontext "Tasks"
       '/// select the Chart placeholder and activate it with [Return] ///'
       LayoutsPreview.TypeKeys ("<RIGHT>",6)
       sleep (1)
       LayoutsPreview.TypeKeys ("<Return>")
       sleep (1)
       kontext "DocumentImpress"
       DocumentImpress.TypeKeys ("<TAB><TAB><TAB>")
       DocumentImpress.TypeKeys ("<Return>")
       DocumentImpress.TypeKeys ("<ESCAPE>")
       sleep (5)

    '/// Save the document in all available formats. ///'
    '/// Close the file. ///'
    '/// Load the different files. ///'
    call fSaveLoadAllFormats (NewFileDir)   'Runs the Function below.

    '/// Delete the different files. ///'
    printlog "    Will try to delete the directory: " + NewFileDir
    app.rmDir NewFileDir

    printlog "tSaveLoadLayoutChart ended."
    '/// End the test ///'
endcase 'tSaveLoadLayoutChart

'****************************************************************************************************

testcase tSaveLoadLayoutSpreadsheet
    Dim NewFileDir as String
    NewFileDir = ConvertPath (gOfficePath + "user\work\LayoutTest\")

    '/// Create New folder in the Work-directory ///'
    printlog "   Will try to create the directory: " + NewFileDir
    app.mkdir NewFileDir

    '/// Create a new document, add a Layout containing a Spreadsheet, Save the document in all available Formats, and open the saved files. ///'

    '/// Make new Presentation ///'
    gApplication = "IMPRESS"
    Call hNewDocument

    '/// Choose and Insert an Layout with a Spreadsheet-field. ///'
    printlog "    Choose and Insert an Layout with a Spreadsheet-field."
    FormatModifyPage
    sleep 1
    kontext "Tasks"
       LayoutsPreview.TypeKeys "<HOME>"
       kontext "Pagelayout_UndoDeleteWarning"
       if Pagelayout_UndoDeleteWarning.exists then
       Pagelayout_UndoDeleteWarning.ok
    end if
    kontext "Tasks"
       '/// select the Spreadsheet placeholder and activate it with [Return] ///'
       LayoutsPreview.TypeKeys ("<RIGHT>",7)
       sleep (1)
       LayoutsPreview.TypeKeys ("<Return>")
       sleep (1)
       kontext "DocumentImpress"
       DocumentImpress.TypeKeys ("<TAB><TAB><TAB>")
       DocumentImpress.TypeKeys ("<Return>")
       DocumentImpress.TypeKeys ("<ESCAPE>")
       sleep (5)

    '/// Save the document in all available formats. ///'
    '/// Close the file. ///'
    '/// Load the different files. ///'
    call fSaveLoadAllFormats (NewFileDir)   'Runs the Function below.

    '/// Delete the different files. ///'
    printlog "   Will try to delete the directory: " + NewFileDir
    app.rmDir NewFileDir  'ConvertPath (gOfficePath + "user\work\LayoutTest\") 

    printlog "tSaveLoadLayoutSpreadsheet ended."
    '/// End the test ///'
endcase 'tSaveLoadLayoutSpreadsheet

'-------------------------------------------------------------------------------

