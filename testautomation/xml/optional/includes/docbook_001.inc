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
'* owner : helge.delfs@oracle.com
'*
'* short description : Library for DocBook XML Im-/Export Test
'*
'\***********************************************************************

sub tDocBookXMLtestdocs
    Dim sSourceDocuments(500) as string
    Dim i as integer, iNumberOfDocuments as integer
    
    printlog "Checking under Tools -> XML Filter Settings that the DocBook XSLT Filter has been installed."    
    printlog "If not install them from ../global/input/xslt_stylesheets/docbook_xhtml.jar"
    if fInitialXMLSetting("docbook") = FALSE then
        warnlog "The DocBook XSLT filter has not been installed!"
        warnlog "Aborting!"
        exit sub
    end if        
    'Calculating how many documents are in that directory (max. 500)
    if FileExists(sSourcePath) then
        'Get the file list and put it in a list (max. 500 per directory!)
        call GetAllFileList(sSourcePath, "*.xml" , sSourceDocuments())  
        iNumberOfDocuments = ListCount(sSourceDocuments())
        printlog iNumberOfDocuments & " DocBook XML files found in " & sSourcePath & "."  
        if iNumberOfDocuments > 0 then
            for i = 2 to iNumberOfDocuments
                'Call the testcase with each document.                                 
                call fLoadDocBookXMLAndSaveIt(sSourceDocuments(i))
            next i
        else
            warnlog "Path " & sSourcePath & " contains no documents!"
        end if
    else
        warnlog "Path " & sSourcePath & " has not been found!"
    end if
end sub

'-------------------------------------------------------------------------

testcase fLoadDocBookXMLAndSaveIt(sSourceDocument as string)            
Dim sExpFileName as string
Dim sFileName as string
 
sFileName = DateiExtract(sSourceDocument)
sExpFileName = sDestinationPath & sFileName
    
    printlog "Load test document " & sFileName & "."
    call hFileOpen (sSourceDocument)    
    printlog "If the document can not be loaded as XML file (wrong Doctype) it will be loaded as ASCII."
    Kontext "AsciiFilterOptionen"
    if AsciiFilterOptionen.Exists(2) then
        AsciiFilterOptionen.Cancel
        QAErrorLog "The DTD may be does not match. Work around is to load it as ASCII file."
        Goto Endsub
    end if    
    printlog "If the Filter Selection dialog occours the  try to select the 'DocBook' entry." 
    Kontext "FilterAuswahl" 
    if FilterAuswahl.Exists(2) then
        QAErrorLog "Filter selection dialog occoured!"
        Filter.Select "DocBook"
        FilterAuswahl.OK
    end if   
    'If there is no document being loaded only the 'backing window' would be visible
    if GetDocumentCount = 0 then
        warnlog "Document " & sFileName & " has not been loaded! It won't be exported! verify manually!" 
    else
        printlog "Save the test document as " & sExpFileName                       
        if hFileSaveAsWithFilterKill(sExpFileName, "DocBook File") = TRUE then            
            printlog "Close the document also if there is a verification dialog."
            call hCloseDocument
            printlog "Check the well formness of the saved document."
            if XMLWellformed (sExpFileName) = FALSE then                                
                warnlog "XML-file " & sExpFileName & " not well formed or does not exist!"
            else                
                printlog "Open the exported document."
                call hFileOpen (sExpFileName)                    
                Kontext "FilterAuswahl" 
                if FilterAuswahl.Exists(2) then
                    QAErrorLog "Filter selection dialog occoured!"
                    Filter.Select "DocBook"
                    FilterAuswahl.OK
                end if
                if GetDocumentCount = 0 then
                    warnlog "EXPORTED document " & sExpFileName & " has not been loaded! It won't be exported! verify manually!" 
                else                    
                    printlog "Close the exported document."
                    call hCloseDocument                        
                end if
            end if
        else
            warnlog "Exporting XML file (" & sExpFileName & ") failed!"                    
        end if
    end if
endcase  

