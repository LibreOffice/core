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
'* owner : wolfram.garten@oracle.com
'*
'* short description : Library 01 for OASIS OpenDocument File Format (Presentation and Drawing) test
'*
'************************************************************************
'*
' #1 talien_attributes                  ' Verification that alian attributes will be stored in document.
' #1 t_all_header_footer                ' http://specs.openoffice.org/impress/header_and_footer.sxw
' #1 t_no_header_footer                 ' http://specs.openoffice.org/impress/header_and_footer.sxw
' #1 t_no_header_footer_master_elements ' http://specs.openoffice.org/impress/header_and_footer.sxw
' #1 t_some_header_footer               ' http://specs.openoffice.org/impress/header_and_footer.sxw
'*
'\***********************************************************************

testcase talien_attributes
    Dim i as integer
    Dim AttrNameInDOM as string
    Dim sPropertyElement as string
    Dim sLogAttributeName as string
    Dim sAttributeSearch(9) as string
    Dim sAttributeValue(9) as string

    printlog "+- Presentation: alien.odp"
    '/// Opening alien.sxi. ///
    call hFileOpen (gTesttoolPath & ConvertPath("xml\required\input\graphics\alien_attributes\alien_attributes_presentation.odp"))
    '/// Saving document. ///
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\graphics\update\alien_attributes_presentation.odp") , "impress8") = TRUE then
        '/// Closing document. ///
        call hCloseDocument
        '/// Unpacking the compressed (ZIP) archive.
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\graphics\update\alien_attributes_presentation.odp") , gOfficePath & ConvertPath("user\work\xml\graphics\update\alien_attributes_presentation\") )
        '/// Verifying the well-formness of styles.xml, content.xml and meta.xml.
        '///+ Only if the three XML files are well formed the next checks (GPF-check, attribute-search) will be started.
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\graphics\update\alien_attributes_presentation\styles.xml")) = TRUE then        
            if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\graphics\update\alien_attributes_presentation\meta.xml")) = TRUE then       
                if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\graphics\update\alien_attributes_presentation\content.xml")) = TRUE then
                    '/// Opening exported document (GPF check). ///
                    call hFileOpen (gOfficePath & ConvertPath("user\work\xml\graphics\update\alien_attributes_presentation.odp")
                    sleep(2)
                    '/// Closing document. ///
                    call hCloseDocument
                    sleep(2)
                    SAXReadFile(gOfficePath & ConvertPath("user\work\xml\graphics\update\alien_attributes_presentation\content.xml"))
                    printlog "-----------------"
                    printlog "Hidden Namespaces"
                    printlog "-----------------"
                    '/// <u><b>content.xml</u></b>
                    '/// <u>Hidden attributes in namespaces</u>
                    '///+ Searching for: xmlns:alien=&quot;http://openoffice.org/2000/alien&quot;
                    '-------------------------------------------------------------------------
                    sAttributeSearch(1) = "xmlns:alien"
                    sAttributeValue(1) = "http://openoffice.org/2000/alien"
                    printlog "++ Feature(s) to be searched for:"
                    printlog " |"
                    '-------------------------------------------------------------------------
                    SAXSeekElement("office:document-content")
                    printlog " +- " & sAttributeSearch(1)
                    if SAXGetAttributeValue(sAttributeSearch(1)) <> sAttributeValue(1) then
                        warnlog "Hidden attributes (in namespace): " & sAttributeSearch(1) & " is not " & sAttributeValue(1) & "!"
                    end if
                    printlog "    * * *"
                    SAXSeekElement("/")
                    '/// <u>hidden attributes</u>
                    '///+<ol><li><b>in a drawing-page style</b></li>
                    '///+<li><b>in a graphics style</b></li>
                    '///+<li><b>in a paragraph style (twice)</b></li>
                    '///+<li><b>in a text style</b></li></ol>
                    '///+ Searching for: alien:key=&quot;page&quot;
                    '///+ Searching for: alien:key=&quot;shape&quot;
                    '///+ Searching for: alien:key=&quot;para&quot;
                    '///+ Searching for: alien:key=&quot;shape-text&quot;
                    '///+ Searching for: alien:key=&quot;text&quot;
                     
                    'The number behind the string variable is the position
                    'in the DOM-tree. The search is hard coded because the
                    'export of the same file should not change in the lifetime
                    'cycle.
                
                    sAttributeSearch(1) = "alien:key"
                    sAttributeValue(1) = "page"
                
                    sAttributeSearch(3) = "alien:key"
                    sAttributeValue(3) = "shape"
                
                    sAttributeSearch(6) = "alien:key"
                    sAttributeValue(6) = "para"
                
                    sAttributeSearch(7) = "alien:key"
                    sAttributeValue(7) = "shape-text"
                
                    sAttributeSearch(9) = "alien:key"
                    sAttributeValue(9) = "text"
                    '-------------------------------------------------------------------------
                    printlog "++ Feature(s) to be searched for:"
                    printlog " |"
                    '-------------------------------------------------------------------------
                    for i = 1 to 9
                        select case i
                            case 1      :   printlog "-----------------------------------------"                    
                                            printlog "Hidden attributes in a drawing-page style"
                                            printlog "-----------------------------------------"
                                            sPropertyElement = "style:drawing-page-properties"
                                            sLogAttributeName = "drawing-page"
                            case 3      :   printlog "-------------------------------------"
                                            printlog "Hidden attributes in a graphics style"
                                            printlog "-------------------------------------"
                                            sPropertyElement = "style:graphic-properties"
                                            sLogAttributeName = "graphics"      
                            case 6, 7   :   printlog "-------------------------------------------"
                                            printlog "Hidden attributes in a paragraph style (" & i-4 & ")"
                                            printlog "-------------------------------------------"
                                            sPropertyElement = "style:paragraph-properties"
                                            sLogAttributeName = "paragraph"       
                            case 9      :   printlog "---------------------------------"
                                            printlog "Hidden attributes in a text style"
                                            printlog "---------------------------------"
                                            sPropertyElement = "style:text-properties"
                                            sLogAttributeName = "text"
                        end select
                        if i = 1 or i = 3 or i = 6 or i = 7 or i = 9 then
                            'The hidden attributes are in a known sequence included,
                            'so the same routine will be used for different styles!
                            SAXSeekElement("/")
                            SAXSeekElement("office:document-content")
                            SAXSeekElement("office:automatic-styles")
                            SAXSeekElement("style:style" , i )
                            AttrNameInDOM = SAXGetAttributeValue("style:name")
                            printlog " + style:style position in DOM: " & i & ". Attribute name: " & AttrNameInDOM
                            printlog " |"       
                
                            'It depends on the type of element (drawing-page, paragraphic, graphics, ...)
                            'which is the next element. That is a difference to OOo XML 1.0 where everything
                            'called 'style:properties'.
                
                            SAXSeekElement(sPropertyElement)
                            printlog " +- " & sAttributeSearch(i)
                            if SAXGetAttributeValue(sAttributeSearch(i)) <> sAttributeValue(i) then          
                                warnlog "Hidden attributes (in a " & sLogAttributeName & " style): " & sAttributeSearch(i) & " is not " & sAttributeValue(i) & "!"
                            end if
                        end if
                    next i
                    SAXSeekElement("/")
                    printlog "    * * *"
                    '-------------------------------------------------------------------------
                    '/// Closing the XML DOM.
                    SAXRelease()
                    '/// Opening the DOM of styles.xml.
                    SAXReadFile(gOfficePath & ConvertPath("user\work\xml\graphics\update\alien_attributes_presentation\styles.xml"))
                    '-------------------------------------------------------------------------
                    printlog "-------------------"
                    printlog "s t y l e s . x m l"
                    printlog "-------------------"
                    printlog "-----------------"
                    printlog "Hidden Namespaces"
                    printlog "-----------------"
                    '/// <u><b>styles.xml</u></b>
                    '/// <u>Hidden attributes in namespaces</u>
                    '///+ Searching for: xmlns:alien=&quot;http://openoffice.org/2000/alien&quot;
                    '-------------------------------------------------------------------------
                    sAttributeSearch(1) = "xmlns:alien"
                    sAttributeValue(1) = "http://openoffice.org/2000/alien"
                    printlog "++ Feature(s) to be searched for:"
                    printlog " |"
                    '-------------------------------------------------------------------------
                    SAXSeekElement("office:document-styles")
                    printlog " +- " & sAttributeSearch(1)
                    if SAXGetAttributeValue(sAttributeSearch(1)) <> sAttributeValue(1) then
                        warnlog "Hidden attributes (in namespace): " & sAttributeSearch(1) & " is not " & sAttributeValue(1) & "!"
                    end if
                    printlog "    * * *"
                    SAXSeekElement("/")
                    printlog "----------------------------------------"
                    printlog "Hidden attributes in a master-page style"
                    printlog "----------------------------------------"
                    '-------------------------------------------------------------------------
                    '/// <u>hidden attributes</u>
                    '///+<ol><li><b>in a master-page style</b></li></ol>
                    '///+ Searching for: alien:key=&quot;master-page&quot;
                    '-------------------------------------------------------------------------
                    sAttributeSearch(1) = "alien:key"
                    sAttributeValue(1) = "master-page"
                    printlog "++ Feature(s) to be searched for:"
                    printlog " |"
                    '-------------------------------------------------------------------------
                    SAXSeekElement("office:document-styles")
                    SAXSeekElement("office:automatic-styles")
                    SAXSeekElement("style:style")
                    SAXSeekElement("style:drawing-page-properties")
                    printlog " +- " & sAttributeSearch(1)
                    if SAXGetAttributeValue(sAttributeSearch(1)) <> sAttributeValue(1) then
                        warnlog "Hidden attributes (in master-page style): " & sAttributeSearch(1) & " is not " & sAttributeValue(1) & "!"
                    end if
                    printlog "    * * *"
                    '-------------------------------------------------------------------------
                    SAXRelease()
                else
                    warnlog "One of the XML files not well formed!"
                end if
            end if
        end if
    else
        warnlog "It was not possible to save the document as XML!"
    end if
endcase

'-------------------------------------------------------------------------

testcase t_all_header_footer
    Dim iPresAttr as integer
    Dim sAttributeSearch(4) as string
    Dim sAttributeValue(4) as string
    Dim sHeaderInNotes as string
    
    printlog "+- Presentation: allheaderfooter_fixed.odp"
    '/// Opening <i>xml\required\input\graphics\header_footer\allheaderfooter_fixed.odp</i>.
    call hFileOpen (gTesttoolPath & ConvertPath("xml\required\input\graphics\header_footer\allheaderfooter_fixed.odp"))
    '/// Saving document as <i>user\work\xml\graphics\update\allheaderfooter_fixed.odp</i> with default presentation document name.
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\graphics\update\allheaderfooter_fixed.odp") , "impress8") = TRUE then
        '/// Closing document.
        call hCloseDocument
        '/// Unpacking the compressed (ZIP) archive. Putting the content to <i>user\work\xml\graphics\update\allheaderfooter_fixed\</i>"
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\graphics\update\allheaderfooter_fixed.odp") , gOfficePath & ConvertPath("user\work\xml\graphics\update\allheaderfooter_fixed\") )
        '/// Verifying the well-formness of styles.xml, content.xml and meta.xml.
        '///+ Only if the three XML files are well formed the next checks (GPF-check, attribute-search) will be started.
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\graphics\update\allheaderfooter_fixed\styles.xml")) = TRUE then        
            if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\graphics\update\allheaderfooter_fixed\meta.xml")) = TRUE then       
                if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\graphics\update\allheaderfooter_fixed\content.xml")) = TRUE then
                    '/// Opening exported document (GPF check). ///
                    call hFileOpen (gOfficePath & ConvertPath("user\work\xml\graphics\update\allheaderfooter_fixed.odp")
                    sleep(2)
                    '/// Closing document. ///
                    call hCloseDocument
                    sleep(2)
                    '/// <u><b>content.xml</u></b>
                    '/// Opening user\work\xml\graphics\update\allheaderfooter_fixed\content.xml with SAX parser.
                    '/// Searching for <ul>
                    '///+<li>&lt;presentation:header-decl presentation:name=&quot;hdr1&quot;&gt;Header in notes/handout  powered by QA&lt;/presentation:header-decl&gt;
                    '///+<li>&lt;presentation:footer-decl presentation:name=&quot;ftr1&quot;&gt;footer powered by QA&lt;/presentation:footer-decl&gt;
                    '///+<li>&lt;presentation:footer-decl presentation:name=&quot;ftr2&quot;&gt;Footer notes/handout  powered by QA&lt;/presentation:footer-decl&gt;
                    '///+<li>&lt;presentation:date-time-decl presentation:name=&quot;dtd1&quot; presentation:source=&quot;fixed&quot;&gt;09.12.2001&lt;/presentation:date-time-decl&gt;</ul>                    
                    sAttributeSearch(1) = "presentation:header-decl"
                    sAttributeValue(1) = "Header in notes/handout  powered by QA"
                    sAttributeSearch(2) = "presentation:footer-decl"
                    sAttributeValue(2) = "footer powered by QA"
                    sAttributeSearch(3) = "presentation:footer-decl"
                    sAttributeValue(3) = "Footer notes/handout  powered by QA"
                    sAttributeSearch(4) = "presentation:date-time-decl"
                    sAttributeValue(4) = "09.12.2001"
                    
                    SAXReadFile(gOfficePath & ConvertPath("user\work\xml\graphics\update\allheaderfooter_fixed\content.xml"))                    
                        for iPresAttr = 1 to 4                        
                            '/// Parsing to office:document-content
                            SAXSeekElement("office:document-content")
                            '/// Parsing to office:body
                            SAXSeekElement("office:body")
                            '/// Parsing to office:presentation                    
                            SAXSeekElement("office:presentation")
                            '/// Parsing to <ol><li>presentation:header-decl</li>
                            '///+ <li>presentation:footer-decl</li>
                            '///+ <li>presentation:footer-decl</li>                           
                            '///+ <li>presentation:date-time-decl</li></ol>
                            if iPresAttr = 3 then
                                'To reach the second 'presentation:footer-decl'-element an additional parameter is needed. 
                                SAXSeekElement(sAttributeSearch(iPresAttr) , 2)
                            else
                                SAXSeekElement(sAttributeSearch(iPresAttr))
                            end if
                            '/// Parsing 1 node deeper (to get the CharNode)                     
                            SAXSeekElement(1)
                            '/// Reading the characters of the CharNode and verifying it against the reference.
                            ' Initializing the variable (empty)
                            sHeaderInNotes = ""                    
                            sHeaderInNotes = SAXGetChars
                            if sHeaderInNotes <> sAttributeValue(iPresAttr) then
                                warnlog "Failed! - Attribute '" & sAttributeSearch(iPresAttr) & "' has the value '" & sHeaderInNotes & "' but it has to be '" & sAttributeValue(iPresAttr) & "'."                                 
                            else
                                printlog "Passed. - Attribute '" & sAttributeSearch(iPresAttr) & "' has the correct value '" & sHeaderInNotes & "'." 
                            end if
                            SAXSeekElement("/")                            
                        next iPresAttr
                    SAXRelease()                    
                else
                    warnlog "One of the XML files not well formed!"
                end if
            end if
        end if
    else
        warnlog "It was not possible to save the document as XML!"
    end if
endcase

'-------------------------------------------------------------------------

testcase t_no_header_footer
    Dim sDrawName as string
    Dim sElementName as string
    Dim sAttributeSearch(3) as string
    Dim sAttributeValue as string
    Dim i as integer
    Dim iChildsOfAutomaticStyles as integer
    printlog "+- Presentation: noheaderfooter.odp"
    '/// Opening <i>xml\required\input\graphics\header_footer\noheaderfooter.odp</i>.
    call hFileOpen (gTesttoolPath & ConvertPath("xml\required\input\graphics\header_footer\noheaderfooter.odp"))
    '/// Saving document as <i>user\work\xml\graphics\update\noheaderfooter.odp</i> with default presentation document name.
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\graphics\update\noheaderfooter.odp") , "impress8") = TRUE then
        '/// Closing document. ///
        call hCloseDocument
        '/// Unpacking the compressed (ZIP) archive.
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\graphics\update\noheaderfooter.odp") , gOfficePath & ConvertPath("user\work\xml\graphics\update\noheaderfooter\") )
        '/// Verifying the well-formness of styles.xml, content.xml and meta.xml.
        '///+ Only if the three XML files are well formed the next checks (GPF-check, attribute-search) will be started.
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\graphics\update\noheaderfooter\styles.xml")) = TRUE then        
            if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\graphics\update\noheaderfooter\meta.xml")) = TRUE then       
                if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\graphics\update\noheaderfooter\content.xml")) = TRUE then
                    '/// Opening exported document (GPF check). ///
                    call hFileOpen (gOfficePath & ConvertPath("user\work\xml\graphics\update\noheaderfooter.odp")
                    sleep(2)
                    '/// Closing document. ///
                    call hCloseDocument                    
                    sleep(2)
                    '/// Opening user\work\xml\graphics\update\noheaderfooter\content.xml with SAX-parser.
                    SAXReadFile(gOfficePath & ConvertPath("user\work\xml\graphics\update\noheaderfooter\content.xml"))
                    '/// <ol> 
                    '///+<li> Parsing to office:document-content
                    SAXSeekElement("office:document-content")
                    '///+ Parsing to office:body
                    SAXSeekElement("office:body")
                    '///+ Parsing to office:presentation                    
                    SAXSeekElement("office:presentation")
                    '///+ Parsing to draw:page
                    SAXSeekElement("draw:page")
                    '///+ Get value of draw:style-name
                    sDrawName = ""
                    sDrawName = SAXGetAttributeValue ("draw:style-name")
                    '///+ Get back to the top of DOM</li>                    
                    SAXSeekElement("/")
                    '///+<li> Parsing to office:document-content
                    SAXSeekElement("office:document-content")
                    '///+ Parsing to office:automatic-styles
                    SAXSeekElement("office:automatic-styles")
                    '///+ Look where the style:style element is where the style:name is the same as it was for draw:name
                    iChildsOfAutomaticStyles = SAXGetChildCount
                    'The count of style:style nodes is normaly 7 but the 8th is a text:list-style where
                    'the searched attributes won't be included.
                    for i = 1 to (iChildsOfAutomaticStyles-1)
                        SAXSeekElement("style:style" , i)
                        sElementName = SAXGetAttributeValue("style:name")
                        if sElementName = sDrawName then
                        '///+ If found the named attribute (e.g. dp1) parsing to style:drawing-page-properties.  
                            SAXSeekElement("style:drawing-page-properties")                            
                            exit for
                        end if
                        if i = iChildsOfAutomaticStyles then
                            '///+ If the style:name is not found a warnlog will be written into the log and the test will be stopped. 
                            warnlog "Failed! The style:style='" & sDrawName & "' has not been found in the automatic-styles! Test will be aborted here!"
                            SAXRelease()
                            goto endsub
                        end if
                        'Go one node to the parent node.
                        SAXSeekElement(0)
                    next i
                    '///+Parsing to <ol><li>presentation:display-footer</li>
                    '///+ <li>presentation:footer-decl</li>
                    '///+ <li>presentation:display-page-number</li>                           
                    '///+ <li>presentation:display-date-time</li></ol>                    
                    sAttributeSearch(1) = "presentation:display-footer"
                    sAttributeSearch(2) = "presentation:display-page-number"
                    sAttributeSearch(3) = "presentation:display-date-time"                   
                    for i = 1 to 3
                        sAttributeValue = SAXGetAttributeValue(sAttributeSearch(i))
                        '///+ Verifying that the value of each attribute is 'false'.</li></ol>
                        if sAttributeValue <> "false" then
                            warnlog "Failed! The value of '" & sAttributeSearch(i) & "' is NOT 'false'!"
                        else
                            printlog "Passed. The value of '" & sAttributeSearch(i) & "' is 'false'."
                        end if
                    next i
                    SAXRelease()
                else
                    warnlog "One of the XML files not well formed!"
                end if
            end if
        end if
    else
        warnlog "It was not possible to save the document as XML!"
    end if
endcase

'-------------------------------------------------------------------------

testcase t_no_header_footer_master_elements
    printlog "+- Presentation: nomasterelements.odp"
    '/// Opening <i>xml\required\input\graphics\header_footer\nomasterelements.odp</i>.
    call hFileOpen (gTesttoolPath & ConvertPath("xml\required\input\graphics\header_footer\nomasterelements.odp"))
    '/// Saving document as <i>user\work\xml\graphics\update\nomasterelements.odp</i> with default presentation document name.
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\graphics\update\nomasterelements.odp") , "impress8") = TRUE then
        '/// Closing document. ///
        call hCloseDocument
        '/// Unpacking the compressed (ZIP) archive.
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\graphics\update\nomasterelements.odp") , gOfficePath & ConvertPath("user\work\xml\graphics\update\nomasterelements\") )
        '/// Verifying the well-formness of styles.xml, content.xml and meta.xml.
        '///+ Only if the three XML files are well formed the next checks (GPF-check, attribute-search) will be started.
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\graphics\update\nomasterelements\styles.xml")) = TRUE then        
            if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\graphics\update\nomasterelements\meta.xml")) = TRUE then       
                if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\graphics\update\nomasterelements\content.xml")) = TRUE then
                    '/// Opening exported document (GPF check). ///
                    call hFileOpen (gOfficePath & ConvertPath("user\work\xml\graphics\update\nomasterelements.odp")
                    sleep(2)
                    '/// Closing document. ///
                    call hCloseDocument
                    sleep(2)
                else
                    warnlog "One of the XML files not well formed!"
                end if
            end if
        end if
    else
        warnlog "It was not possible to save the document as XML!"
    end if
endcase

'-------------------------------------------------------------------------

testcase t_some_header_footer
    Dim sDrawName as string
    Dim sElementName as string
    Dim sAttributeSearch(4) as string
    Dim sAttributeValue as string
    Dim i as integer
    Dim iChildsOfAutomaticStyles as integer
    Dim iLoopSearch as integer
    printlog "+- Presentation: someheaderfooter_varible.odp"
    '/// Opening <i>xml\required\input\graphics\header_footer\someheaderfooter_varible.odp</i>.
    call hFileOpen (gTesttoolPath & ConvertPath("xml\required\input\graphics\header_footer\someheaderfooter_varible.odp"))
    '/// Saving document as <i>user\work\xml\graphics\update\someheaderfooter_varible.odp</i> with default presentation document name.
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\graphics\update\someheaderfooter_varible.odp") , "impress8") = TRUE then
        '/// Closing document. ///
        call hCloseDocument
        '/// Unpacking the compressed (ZIP) archive.
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\graphics\update\someheaderfooter_varible.odp") , gOfficePath & ConvertPath("user\work\xml\graphics\update\someheaderfooter_varible\") )
        '/// Verifying the well-formness of styles.xml, content.xml and meta.xml.
        '///+ Only if the three XML files are well formed the next checks (GPF-check, attribute-search) will be started.
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\graphics\update\someheaderfooter_varible\styles.xml")) = TRUE then        
            if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\graphics\update\someheaderfooter_varible\meta.xml")) = TRUE then       
                if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\graphics\update\someheaderfooter_varible\content.xml")) = TRUE then
                    '/// Opening exported document (GPF check). ///
                    call hFileOpen (gOfficePath & ConvertPath("user\work\xml\graphics\update\someheaderfooter_varible.odp")
                    sleep(2)
                    '/// Closing document. ///
                    call hCloseDocument
                    sleep(2)
                    '/// Opening user\work\xml\graphics\update\noheaderfooter\content.xml with SAX-parser.
                    SAXReadFile(gOfficePath & ConvertPath("user\work\xml\graphics\update\someheaderfooter_varible\content.xml"))
                    '/// <ol> 
                    '///+<li> Parsing to office:document-content
                    For iLoopSearch = 1 to 2
                        SAXSeekElement("office:document-content")
                        '///+ Parsing to office:body
                        SAXSeekElement("office:body")
                        '///+ Parsing to office:presentation                    
                        SAXSeekElement("office:presentation")
                        '///+ Parsing to draw:page
                        SAXSeekElement("draw:page")
                        '///+ In second loop enter <i>presentation:notes</i> node
                        if iLoopSearch = 2 then
                            SAXSeekElement("presentation:notes")
                        end if
                        '///+ Get value of draw:style-name
                        sDrawName = ""
                        sDrawName = SAXGetAttributeValue ("draw:style-name")
                        '///+ Get back to the top of DOM                    
                        SAXSeekElement("/")
                        '///+<li> Parsing to office:document-content
                        SAXSeekElement("office:document-content")
                        '///+ Parsing to office:automatic-styles
                        SAXSeekElement("office:automatic-styles")
                        '///+ Look where the style:style element is where the style:name is the same as it was for draw:name
                        iChildsOfAutomaticStyles = SAXGetChildCount
                        'The count of style:style nodes is normaly 7 but the 8th is a text:list-style where
                        'the searched attributes won't be included.
                        for i = 1 to (iChildsOfAutomaticStyles-1)
                            SAXSeekElement("style:style" , i)
                            sElementName = SAXGetAttributeValue("style:name") 
                            if sElementName = sDrawName then
                            '///+ If found the named attribute (e.g. dp1) parsing to style:drawing-page-properties. 
                                SAXSeekElement("style:drawing-page-properties")                            
                                exit for
                            end if
                            if i = iChildsOfAutomaticStyles then
                                '///+ If the style:name is not found a warnlog will be written into the log and the test will be stopped. 
                                warnlog "Failed! The style:style='" & sDrawName & "' has not been found in the automatic-styles! Test will be aborted here!"
                                SAXRelease()
                                goto endsub
                            end if
                            'Go one node to the parent node.
                            SAXSeekElement(0)
                        next i                       
                        if iLoopSearch = 1 then
                            '///+Parsing to <ol>                        
                            '///+ <li>presentation:display-footer</li>
                            '///+ <li>presentation:footer-decl</li>
                            '///+ <li>presentation:display-page-number</li>                           
                            '///+ <li>presentation:display-date-time</li></ol>                    
                            sAttributeSearch(1) = "presentation:display-footer"
                            sAttributeSearch(2) = "presentation:display-page-number"
                            sAttributeSearch(3) = "presentation:display-date-time"                        
                            for i = 1 to 3
                                sAttributeValue = SAXGetAttributeValue(sAttributeSearch(i))                         
                                '/// Verifying that the value of <i>presentation:display-footer</i> attribute is 'false'.
                                if i = 1 then
                                    if sAttributeValue <> "false" then
                                        warnlog "Failed! The value of '" & sAttributeSearch(i) & "' is NOT 'false'!"
                                    else
                                        printlog "Passed. The value of '" & sAttributeSearch(i) & "' is 'false'."
                                    end if
                                else
                                '///+ Verifying that the values of <i>presentation:display-page-number</i> and <i>presentation:display-date-time</i> attribute are 'true'.                        
                                    if sAttributeValue <> "true" then
                                        warnlog "Failed! The value of '" & sAttributeSearch(i) & "' is NOT 'true'!"
                                    else
                                        printlog "Passed. The value of '" & sAttributeSearch(i) & "' is 'true'."
                                    end if                        
                                end if                                
                            next i
                            '///+ Get back to the top of DOM                    
                            SAXSeekElement("/")                            
                        end if
                        if iLoopSearch = 2 then
                            '///+Parsing to second page-style attributes <ol>                        
                            '///+ <li>presentation:display-header</li>
                            '///+ <li>presentation:display-footer</li>
                            '///+ <li>presentation:display-page-number</li>                           
                            '///+ <li>presentation:display-date-time</li></ol>                    
                            sAttributeSearch(1) = "presentation:display-header"
                            sAttributeSearch(2) = "presentation:display-footer"
                            sAttributeSearch(3) = "presentation:display-page-number"
                            sAttributeSearch(4) = "presentation:display-date-time"
                            for i = 1 to 4
                                sAttributeValue = SAXGetAttributeValue(sAttributeSearch(i))                         
                                '/// Verifying that the value of <i>presentation:display-page-number</i> and <i>presentation:display-date-time</i> attributes is 'false'.
                                if i = 3 OR i = 4 then
                                    if sAttributeValue <> "false" then
                                        warnlog "Failed! The value of '" & sAttributeSearch(i) & "' is NOT 'false'!"
                                    else
                                        printlog "Passed. The value of '" & sAttributeSearch(i) & "' is 'false'."
                                    end if
                                else
                                '///+ Verifying that the values of <i>presentation:display-header</i> and <i>presentation:display-footer</i> attribute are 'true'.                        
                                    if sAttributeValue <> "true" then
                                        warnlog "Failed! The value of '" & sAttributeSearch(i) & "' is NOT 'true'!"
                                    else
                                        printlog "Passed. The value of '" & sAttributeSearch(i) & "' is 'true'."
                                    end if                        
                                end if                                
                            next i                        
                        end if
                    next iLoopSearch
                    SAXRelease()
                else
                    warnlog "One of the XML files not well formed!"
                end if
            end if
        end if
    else
        warnlog "It was not possible to save the document as XML!"
    end if
endcase

'-------------------------------------------------------------------------

