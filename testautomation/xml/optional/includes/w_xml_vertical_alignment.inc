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
'* short description : testing vertical alignment in writer 
'*
'*************************************************************************************************
'*
' #1 tVerticalAlignmentWriter  'Testing for vertical alignment in writer
'*
'\*************************************************************************************************

testcase tVerticalAlignmentWriter
    ' Dimensioning variables
    dim iSaxSeekIndex as INTEGER        'counter-variable for searching through the styles:styles
    dim sSaveAsFile as STRING           'TestDoc is going to be saved with this name
    dim sWriterContentXML as STRING     'Location of the content.xml
    dim sLoadFileName as STRING         'Variable for name of the Testdoc
    dim sUnpackStorageDir as STRING     'Name of the direcotry where the xml file is being unpacked
    dim sAttrValue as STRING            'Name of the variable to store the searched value in
    Dim xXMLStyleName as STRING         'Draw:Style-name to be searched for
    Dim iChildCount as INTEGER          'Counter for SAX Child nodes
    
    ' Setting paths and variables
    sSaveAsFile = gOfficePath & ConvertPath("user\work\VerticalAlignment.odt")
    printlog "Save-path is " & sSaveAsFile
    sUnpackStorageDir = gOfficePath & ConvertPath("user\work\VerticalAlignment")
    printlog "UnpackStorageDir is " & sUnpackStorageDir    
    printlog "Used Save-As Filter is " & gWriterFilter
    sWriterContentXML = gOfficePath & ConvertPath("user\work\VerticalAlignment\content.xml")
    printlog "ContentXML File is to be found in " & sWriterContentXML
    sLoadFileName = ConvertPath (gTesttoolPath + "xml\optional\input\writer\verttoline1.doc")
    printlog "The used TestDoc is " & sLoadFileName
                
    printlog "Checking if storage Directory is empty"
    if Dir(sUnpackStorageDir) <> "" then
        printlog "UnpackStorageDir was not empty, deleting."
        RmDir (sUnpackStorageDir)
    else
        printlog "UnpackStorageDir is emtpy."
    endif
        
    printlog "Loading test document gTesttoolPath\xml\optional\input\writer\verttoline1.doc"
    if hFileOpen (sLoadFileName) = FALSE then
        warnlog "Loading File failed!"
    else                        
        printlog "Save document in ODF file format."
        if hFileSaveAsWithFilterKill (sSaveAsFile , "writer8") = FALSE then
            warnlog "Saving " & sSaveAsFile & " failed! -> Exiting test!"
            call hCloseDocument
        else
            call hCloseDocument
        endif
    endif        
    printlog "Unzip or unjar the gOfficePath/user/work/VerticalAlignment.odt into the directory gOfficePath/user/work/VerticalAlignment"
    UnpackStorage(sSaveAsFile, sUnpackStorageDir)           
    printlog "Use a XML editor like jEdit and open the structure of the XML file gOfficePath/user/work/VerticalAlignment/Object 1/content.xml"
    printlog "opening XML file and creating DOM."
    SAXReadFile(sWriterContentXML)
    sleep(2)
    printlog "Go to the root node."
    SAXSeekElement("/")        
    printlog "Get the Draw style name to search the attribute in..."    
    printlog "office:document-content"
    printlog "Search for draw:style-name to be used in..."
    SAXSeekElement("office:document-content")    
    printlog "office:body"
    SAXSeekElement("office:body")    
    printlog "office:text"
    SAXSeekElement("office:text")    
    printlog "text:p"
    SAXSeekElement("text:p" , 1)    
    printlog "draw:frame"
    SAXSeekElement("draw:frame" , 1)    
    printlog "draw:style-name"
    xXMLStyleName = SAXGetAttributeValue("draw:style-name")
    printlog "draw:style-name to search for is " & xXMLStyleName & "."    
    printlog "Search for: style:vertical-pos=&quot;from-top&quot;"       
    'Setting parser back to root node
    SAXSeekElement("/")        
    printlog "Seek office:document-content."
    SAXSeekElement("office:document-content")        
    printlog "Seek office:automatic-styles."
    SAXSeekElement("office:automatic-styles")        
    printlog "Search through style:style elements."        
    printlog "...in one of the 4 style:style elements there should be a..."
    iChildCount = SAXGetChildCount
    'printlog "There are " & iChildCount & " Childnodes."
    for iSaxSeekIndex = 1 to iChildCount
        SAXSeekElement(iSaxSeekIndex)
        if SAXGetAttributeValue("style:name") = xXMLStyleName then
            SAXSeekElement("style:graphic-properties")                
            printlog "style:graphic-properties with the value from-top"            
            printlog "Seeking for style:vertical-pos."
            if SaxGetAttributeName(5) = "style:vertical-pos" then
                'printlog "Found attribute 'style:vertical-pos' in childnode " & iSaxSeekIndex & "."
                printlog "Get value from style:vertical-pos."
                sAttrValue = SAXGetAttributeValue("style:vertical-pos")
                'printlog "Value is " & sAttrValue & "."
                'lcase(sAttrValue)
                if sAttrValue = "from-top" then
                    printlog "Expected value of 'style:vertical-pos' = from-top was found."                 
                    exit for
                else
                    warnlog "Expected value of 'style:vertical-pos' = from-top was not found!"
                endif
                
            else
                warnlog "Attribut style:vertical-pos not found in element " & iSaxSeekIndex & "."
            endif
            SAXSeekElement(0) 
        endif
        SAXSeekElement(0)
    next iSaxSeekIndex
    SAXRelease
endcase
