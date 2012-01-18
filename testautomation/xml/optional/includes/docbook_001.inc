'encoding UTF-8  Do not remove or change this line!
'*************************************************************************
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
'*************************************************************************
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

