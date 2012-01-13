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
'* short description: Save & Load testing of Layout -templates.
'*
'\**************************************************************************************

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
