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
'* short description : XML Writer Include File
'*
'\***********************************************************************************
   Dim Isliste(250) as string
   Dim OutputPath as string

sub sxw7_01

   printlog "-------------------------- SXW7_01.INC ------------------------------"

   printlog "------------------------ Alien Attributes ---------------------------"
   call tfiscus
   call tfiscus2
   
   printlog "---------------------- File Format Changes --------------------------"
   call tfeat476
   call tfeat1318

   printlog "---------------------------------------------------------------------"
   call tcharacters 

end sub
'
'-------------------------------------------------------------------------
'
testcase tfiscus
    Dim i as integer
    Dim a as integer
    Dim xXMLStyleName as string
    Dim xPath as string
    Dim AttributeSearch(50) as string
    Dim AttributeValue(50) as string
    Dim iStylesToCount as integer
    Dim sStyleStyleValueA as string
 
    printlog "+- fiscus.sxw"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\writer\fiscus.sxw"))
    '/// Saving (and if already exiting overwriting) the document.
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\writer\level1\fiscus.sxw") , "StarOffice XML (Writer)") then
        '/// Closing the document also if there is a verification dialog.
        call hCloseDocument()
        sleep(3)     
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\writer\level1\fiscus.sxw") , gOfficePath & ConvertPath("user\work\xml\writer\level1\fiscus") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\writer\level1\fiscus\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\writer\level1\fiscus\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\writer\level1\fiscus\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\writer\level1\fiscus.sxw"))
            sleep(2)
            call hCloseDocument
            sleep(2)
            SAXReadFile(gOfficePath & ConvertPath("user\work\xml\writer\level1\fiscus\content.xml"))
            printlog "    ---------------------"
            printlog "    c o n t e n t . x m l"
            printlog "    ---------------------"

            printlog "-----------------"
            printlog "Hidden Namespaces"
            printlog "-----------------"
            '/// <u><b>content.xml</u></b>
            '/// <u>Hidden Namespaces</u>
            '///+ Searching for: xmlns:foo=&quot;http://www.foo.com&quot;
            '///+ Searching for: xmlns:bar=&quot;http://www.bar.com&quot; ///
            '-------------------------------------------------------------------------
            AttributeSearch(1) = "xmlns:foo"
            AttributeValue(1) = "http://www.foo.com"

            AttributeSearch(2) = "xmlns:bar"
            AttributeValue(2) = "http://www.bar.com"
            printlog "++ Feature(s) to be searched for:"
            printlog " |"
            '-------------------------------------------------------------------------
            SAXSeekElement("office:document-content")
            for i = 1 to 2
                printlog " +- " & AttributeSearch(i)
                if SAXGetAttributeValue(AttributeSearch(i)) <> AttributeValue(i) then
                    warnlog "Hidden attributes (in namespace): " & AttributeSearch(i) & " is not " & AttributeValue(i) & "!"
                end if
            next i
            SAXSeekElement("/")
            printlog "    * * *"
            '/// <u>hidden attributes <b>in a table style</b></u>
            '///+ Searching for: attr1=&quot;42&quot;
            '///+ Searching for: foo:attr2=&quot;43&quot;
            '///+ Searching for: bar:attr3=&quot;44&quot; ///
            '/// <u>hidden attributes <b>in a table cell style</b></u>
            '///+ Searching for: attr1=&quot;42&quot;
            '///+ Searching for: foo:attr2=&quot;43&quot;
            '///+ Searching for: bar:attr3=&quot;44&quot; ///
            '/// <u>hidden attributes <b>in a paragraph style</b></u>
            '///+ Searching for: attr1=&quot;42&quot;
            '///+ Searching for: foo:attr2=&quot;43&quot;
            '///+ Searching for: bar:attr3=&quot;44&quot; ///
            '/// <u>hidden attributes <b>in a text style</b></u>
            '///+ Searching for: attr1=&quot;42&quot;
            '///+ Searching for: foo:attr2=&quot;43&quot;
            '///+ Searching for: bar:attr3=&quot;44&quot; ///
            '/// <u>hidden attributes <b>in a frame style</b></u>
            '///+ Searching for: attr1=&quot;42&quot;
            '///+ Searching for: foo:attr2=&quot;43&quot;
            '///+ Searching for: bar:attr3=&quot;44&quot; ///
            '-------------------------------------------------------------------------
            AttributeSearch(1) = "attr1"
            AttributeValue(1) = "42"

            AttributeSearch(2) = "foo:attr2"
            AttributeValue(2) = "43"

            AttributeSearch(3) = "bar:attr3"
            AttributeValue(3) = "44"
            '-------------------------------------------------------------------------
            for a = 1 to 8
                if a = 1 then
                    printlog "----------------------------------"
                    printlog "Hidden attributes in a table style"
                    printlog "----------------------------------"
                end if
                if a = 4 then
                    printlog "---------------------------------------"
                    printlog "Hidden attributes in a table cell style"
                    printlog "---------------------------------------"
                end if
                if a = 6 then
                    printlog "--------------------------------------"
                    printlog "Hidden attributes in a paragraph style"
                    printlog "--------------------------------------"
                end if
                if a = 7 then
                    printlog "---------------------------------"
                    printlog "Hidden attributes in a text style"
                    printlog "---------------------------------"
                end if
                if a = 8 then
                    printlog "----------------------------------"
                    printlog "Hidden attributes in a frame style"
                    printlog "----------------------------------"
                end if
                if a = 1 or a = 4 or a = 6 or a = 7 or a = 8 then
                    SAXSeekElement("office:document-content")
                    SAXSeekElement("office:automatic-styles")
                    SAXSeekElement("style:style" , a)
                    SAXSeekElement("style:properties")
                    printlog "++ Feature(s) to be searched for:"
                    printlog " |"
                    for i = 1 to 3
                        printlog " +- " & AttributeSearch(i)
                        if SAXGetAttributeValue(AttributeSearch(i)) <> AttributeValue(i) then
                            if a = 1 then 
                                warnlog "Hidden attributes (in a table style): " & AttributeSearch(i) & " is not " & AttributeValue(i) & "!"
                            end if
                            if a = 4 then 
                                warnlog "Hidden attributes (in a table cell style): " & AttributeSearch(i) & " is not " & AttributeValue(i) & "!"
                            end if
                            if a = 6 then 
                                warnlog "Hidden attributes (in a paragraph style): " & AttributeSearch(i) & " is not " & AttributeValue(i) & "!"
                            end if
                            if a = 7 then
                                warnlog "#i74125# - Alien attributes get lost in text style properties." 
                                ' warnlog "Hidden attributes (in a text style): " & AttributeSearch(i) & " is not " & AttributeValue(i) & "!"
                            end if
                            if a = 8 then 
                                warnlog "Hidden attributes (in a frame style): " & AttributeSearch(i) & " is not " & AttributeValue(i) & "!"
                            end if
                        end if
                    next i
                    printlog "    * * *"
                end if
                SAXSeekElement("/")
            next a
            SAXRelease()
            SAXReadFile(gOfficePath & ConvertPath("user\work\xml\writer\level1\fiscus\styles.xml"))
            '-------------------------------------------------------------------------
            printlog "    -------------------"
            printlog "    s t y l e s . x m l"
            printlog "    -------------------"
            printlog "-----------------"
            printlog "Hidden Namespaces"
            printlog "-----------------"
            '/// <u><b>STYLES.XML</u></b>
            '/// <u>Hidden Namespace</u>
            '///+ Searching for: xmlns:foo=&quot;http://www.foo.com&quot;
            '///+ Searching for: xmlns:bar=&quot;http://www.bar.com&quot; ///
            '-------------------------------------------------------------------------
            AttributeSearch(1) = "xmlns:foo"
            AttributeValue(1) = "http://www.foo.com"

            AttributeSearch(2) = "xmlns:bar"
            AttributeValue(2) = "http://www.bar.com"
            printlog "++ Feature(s) to be searched for:"
            printlog " |"
            '-------------------------------------------------------------------------
            SAXSeekElement("office:document-styles")
            for i = 1 to 2
                printlog " +- " & AttributeSearch(i)
                if SAXGetAttributeValue(AttributeSearch(i)) <> AttributeValue(i) then
                    warnlog "Hidden attributes (in namespace): " & AttributeSearch(i) & " is not " & AttributeValue(i) & "!"
                end if
            next i
            SAXSeekElement("/")
            '-------------------------------------------------------------------------
            printlog "    * * *"
            printlog "-----------------------------------------------"
            printlog "Hidden attributes in TEMPLATE STYLE 'Text body'"
            printlog "-----------------------------------------------"
            '/// <u>hidden attributes <b>in the template style &acute;Text body&acute;</b></u>
            '///+ Searching for: attr1=&quot;42&quot;
            '///+ Searching for: foo:attr2=&quot;43&quot;
            '///+ Searching for: bar:attr3=&quot;44&quot; ///
            '-------------------------------------------------------------------------
            AttributeSearch(1) = "attr1"
            AttributeValue(1) = "42"

            AttributeSearch(2) = "foo:attr2"
            AttributeValue(2) = "43"

            AttributeSearch(3) = "bar:attr3"
            AttributeValue(3) = "44"

            printlog "++ Feature(s) to be searched for:"
            printlog " |"
            '-------------------------------------------------------------------------
            SAXSeekElement("office:document-styles")
            SAXSeekElement("office:styles")
            ' There are 7 style:style elements; it is important to get
            ' the 'Text body' titled one because it is the onliest which
            ' has one additional 'style:properties' element.
            ' Executing SAXSeekElement without having an additional
            ' element interrupts the test run!
            for iStylesToCount = 1 to 7
                SAXSeekElement("style:style", iStylesToCount)
                sStyleStyleValueA = SAXGetAttributeValue("style:name")
                if sStyleStyleValueA = "Text body" then
                    exit for
                else 
                    SAXSeekElement(0)  
                end if
            next iStylesToCount
            ' Go into the style:propries element in 'Text body' named style:style element.
            ' There the three hidden attributes should be stored.
            SAXSeekElement("style:properties")
            for i = 1 to 3
                printlog " +- " & AttributeSearch(i)
                if SAXGetAttributeValue(AttributeSearch(i)) <> AttributeValue(i) then
                    warnlog "Hidden attributes (in a template style): " & AttributeSearch(i) & " is not " & AttributeValue(i) & "!"
                end if
            next i
            '-------------------------------------------------------------------------
            printlog "    * * *"
            SAXRelease()
        end if
    else
        call hCloseDocument
    end if
endcase
'
'-------------------------------------------------------------------------
'
testcase tfiscus2
    Dim i as integer
    Dim xXMLStyleName as string
    Dim AttributeSearch(50) as string
    Dim AttributeValue(50) as string
    printlog "+- fiscus2.sxw"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\writer\fiscus2.sxw"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\writer\level1\fiscus2.sxw") , "StarOffice XML (Writer)") then
        '/// Closing the document also if there is a verification dialog.
        call hCloseDocument()
        sleep(3)     
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\writer\level1\fiscus2.sxw") , gOfficePath & ConvertPath("user\work\xml\writer\level1\fiscus2") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\writer\level1\fiscus2\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\writer\level1\fiscus2\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\writer\level1\fiscus2\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\writer\level1\fiscus2.sxw"))
            sleep(2)
            call hCloseDocument
            sleep(2)
            SAXReadFile(gOfficePath & ConvertPath("user\work\xml\writer\level1\fiscus2\content.xml"))
            printlog "    ---------------------"
            printlog "    c o n t e n t . x m l"
            printlog "    ---------------------"

            printlog "-----------------"
            printlog "Hidden Namespaces"
            printlog "-----------------"
            '/// <u><b>content.xml</u></b>
            '/// <u>Hidden Namespaces</u>
            '///+ Searching for: xmlns:foo=&quot;http://www.foo.com&quot;
            '///+ Searching for: xmlns:bar=&quot;http://www.bar.com&quot; ///
            '-------------------------------------------------------------------------
            AttributeSearch(1) = "xmlns:foo"
            AttributeValue(1) = "http://www.foo.com"

            AttributeSearch(2) = "xmlns:bar"
            AttributeValue(2) = "http://www.bar.com"
            printlog "++ Feature(s) to be searched for:"
            printlog " |"
            '-------------------------------------------------------------------------
            SAXSeekElement("office:document-content")
            for i = 1 to 2
                printlog " +- " & AttributeSearch(i)
                if SAXGetAttributeValue(AttributeSearch(i)) <> AttributeValue(i) then
                    warnlog "Hidden attributes (in namespace): " & AttributeSearch(i) & " is not " & AttributeValue(i) & "!"
                end if
            next i
            SAXSeekElement("/")
            printlog "    * * *"
            printlog "--------------------------------------------------------------------------"
            printlog "Hidden attributes in text style but a 'span'-item over the whole paragraph"
            printlog "--------------------------------------------------------------------------"
            '/// <u>hidden attributes <b>in a text style</b></u>
            '///+ where a 'span'-item exists in a whole paragraph.
            '///+ Searching for: attr1=&quot;42&quot;
            '///+ Searching for: foo:attr2=&quot;43&quot;
            '///+ Searching for: bar:attr3=&quot;44&quot; ///
            '-------------------------------------------------------------------------
            AttributeSearch(1) = "attr1"
            AttributeValue(1) = "42"

            AttributeSearch(2) = "foo:attr2"
            AttributeValue(2) = "43"

            AttributeSearch(3) = "bar:attr3"
            AttributeValue(3) = "44"

            printlog "++ Feature(s) to be searched for:"
            printlog " |"
            '-------------------------------------------------------------------------
            SAXSeekElement("office:document-content")
            SAXSeekElement("office:automatic-styles")
            SAXSeekElement("style:style" , 9)
            SAXSeekElement("style:properties")
            for i = 1 to 3
                printlog " +- " & AttributeSearch(i)
                if SAXGetAttributeValue(AttributeSearch(i)) <> AttributeValue(i) then
                    warnlog "Hidden attributes (in a text style with a 'span'-item over the whole paragraph): " & AttributeSearch(i) & " is not " & AttributeValue(i) & "!"
                end if
            next i
            printlog "    * * *"
            SAXRelease()
        end if
    else
        call hCloseDocument
    end if
endcase
'
'-------------------------------------------------------------------------
'
testcase tfeat476
    Dim i as integer
    Dim a as integer
    Dim xXMLStyleName as string
    Dim xPath as string
    Dim AttributeSearch(50) as string
    Dim AttributeValue(50) as string
    printlog "+- feat476.sxw"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\writer\feat476.sxw"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\writer\level1\feat476.sxw") , "StarOffice XML (Writer)") then
        '/// Closing the document also if there is a verification dialog.
        call hCloseDocument()
        sleep(3)     
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\writer\level1\feat476.sxw") , gOfficePath & ConvertPath("user\work\xml\writer\level1\feat476") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\writer\level1\feat476\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\writer\level1\feat476\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\writer\level1\feat476\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\writer\level1\feat476.sxw"))
            sleep(2)
            call hCloseDocument
            sleep(2)
            SAXReadFile(gOfficePath & ConvertPath("user\work\xml\writer\level1\feat476\content.xml"))
            printlog "    ---------------------"
            printlog "    c o n t e n t . x m l"
            printlog "    ---------------------"

            printlog "-----------------"
            printlog "Vertical Relation"
            printlog "-----------------"
            '/// <u><b>content.xml</u></b>
            '/// <u>Vertical Relation</u>
            '///+ <blockquote>The <i>style:vertical-rel</i> attribute has a new value "text".
            '///+ It is used by frames (i.e. images, text-boxes, etc.) that
            '///+ are bound as characters to specify a vertical alignment
            '///+ relative to the text height of the line that contains the
            '///+ frame.
            '///+ Before this change, the value "char" was used for this
            '///+ purpose. This lead to an ambiguity, because the same value
            '///+ is used with a different meaning for frames bound at a character.</blockquote>
            '/// Searching for: style:vertical-rel=&quot;text&quot;

            '-------------------------------------------------------------------------
            AttributeSearch(1) = "style:vertical-rel"
            AttributeValue(1) = "text"
            printlog "++ Feature(s) to be searched for:"
            printlog " |"
            '-------------------------------------------------------------------------
            SAXSeekElement("office:document-content")
            SAXSeekElement("office:automatic-styles")
            SAXSeekElement("style:style")
            SAXSeekElement("style:properties")
            for i = 1 to 1
                printlog " +- " & AttributeSearch(i)
                if SAXGetAttributeValue(AttributeSearch(i)) <> AttributeValue(i) then
                    warnlog "Vertical Relation (XML file format change 476): " & AttributeSearch(i) & " is not " & AttributeValue(i) & "!"
                end if
            next i
            SAXSeekElement("/")
            printlog "    * * *"
            SAXRelease()
        end if
    else
        call hCloseDocument
    end if
endcase
'
'-------------------------------------------------------------------------
'
testcase tfeat1318
    Dim i as integer
    Dim a as integer
    Dim xXMLStyleName as string
    Dim xPath as string
    Dim AttributeSearch(50) as string
    Dim AttributeValue(50) as string
    printlog "+- feat1318.sxw"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\writer\feat1318.sxw"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\writer\level1\feat1318.sxw") , "StarOffice XML (Writer)") then
        '/// Closing the document also if there is a verification dialog.
        call hCloseDocument()
        sleep(3)     
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\writer\level1\feat1318.sxw") , gOfficePath & ConvertPath("user\work\xml\writer\level1\feat1318") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\writer\level1\feat1318\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\writer\level1\feat1318\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\writer\level1\feat1318\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\writer\level1\feat1318.sxw"))
            sleep(2)
            call hCloseDocument
            sleep(2)
            SAXReadFile(gOfficePath & ConvertPath("user\work\xml\writer\level1\feat1318\content.xml"))
            printlog "    ---------------------"
            printlog "    c o n t e n t . x m l"
            printlog "    ---------------------"

            printlog "-----------------"
            printlog "Vertical Position"
            printlog "-----------------"
            '/// <u><b>content.xml</u></b>
            '/// <u>Vertical Position</u>
            '///+ <blockquote>The <i>style:vertical-pos</i> attribute that can be attached to the
            '///+ <i><style:properties></i> element of graphic styles has got the new
            '///+ value &quot;below&quot;. It can be used for frames that are bound to a
            '///+ character only and means that the frame is displayed right
            '///+ below the anchor character.
            '///+ The meaning of any other value of the <i>style:vertical-pos</i>
            '///+ attribute is unchanged. There also was no other way to set
            '///+ the vertical position of a frame to &quot;below the anchor
            '///+ character&quot;.</blockquote>
            '/// Searching for: style:vertical-pos=&quot;below&quot;

            '-------------------------------------------------------------------------
            AttributeSearch(1) = "style:vertical-pos"
            AttributeValue(1) = "below"
            printlog "++ Feature(s) to be searched for:"
            printlog " |"
            '-------------------------------------------------------------------------
            SAXSeekElement("office:document-content")
            SAXSeekElement("office:automatic-styles")
            SAXSeekElement("style:style")
            SAXSeekElement("style:properties")
            for i = 1 to 1
                printlog " +- " & AttributeSearch(i)
                if SAXGetAttributeValue(AttributeSearch(i)) <> AttributeValue(i) then
                    warnlog "Vertical Relation (XML file format change 1318): " & AttributeSearch(i) & " is not " & AttributeValue(i) & "!"
                end if
            next i
            SAXSeekElement("/")
            printlog "    * * *"
            SAXRelease()
        end if
    else
        call hCloseDocument
    end if
endcase
'
'-------------------------------------------------------------------------
'
testcase tcharacters
    Dim a as integer
    Dim b as integer
    Dim c as integer
    Dim g as integer
    Dim i as integer
    Dim xXMLStyleName as string
    Dim xPath as string
    Dim  AttributeSearch(50) as string
    Dim AttributeValue(50) as string
    printlog "+- characters.sdw"
    '/// Opening the characters.sdw file from the input directory.
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\writer\characters.sdw"))
    '/// Saving the document in OOo XML 1.0 file format.
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\writer\level1\characters.sxw") , "StarOffice XML (Writer)") then
        '/// Closing the document also if there is a verification dialog.
        call hCloseDocument()
        sleep(3)
        '/// Unpack the complete file (ZIP archieve) into a seperate directory.
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\writer\level1\characters.sxw") , gOfficePath & ConvertPath("user\work\xml\writer\level1\characters") )
        '/// Verifying the wellformness of content.xml, styles.xml and meta.xml.
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\writer\level1\characters\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\writer\level1\characters\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\writer\level1\characters\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            '/// Opening the exported document (GPF-check).
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\writer\level1\characters.sxw"))
            sleep(2)
            '/// Closing the document.
            call hCloseDocument
            sleep(2)
            SAXReadFile(gOfficePath & ConvertPath("user\work\xml\writer\level1\characters\content.xml"))
            printlog "    ---------------------"
            printlog "    c o n t e n t . x m l"
            printlog "    ---------------------"

            printlog "----------"
            printlog "Paragraphs"
            printlog "----------"
            '-------------------------------------------------------------------------
            '/// <u><b>CONTENT.XML</u></b>
            '/// <u>Paragraph Style P2</u>
            SAXSeekElement("office:document-content")
            SAXSeekElement("office:body")
            SAXSeekElement("text:p" , 15)
            xXMLStyleName = SAXGetAttributeValue("text:style-name")
            printlog "text:style-name: " & xXMLStyleName
            '///+ Searching for: style:text-position=&quot;super 90%&quot;
            '///+ Searching for: style:text-blinking=&quot;true&quot; ///
            AttributeSearch(1) = "style:text-position"
            AttributeValue(1) = "super 90%"

            AttributeSearch(2) = "style:text-blinking"
            AttributeValue(2) = "true"
            printlog "++ Feature(s) to be searched for:"
            printlog " |"
            SAXSeekElement("/")
            SAXSeekElement("office:document-content")
            SAXSeekElement("office:automatic-styles")
            a = SAXGetChildCount
            for c = 1 to a
                SAXSeekElement(c)
                if SAXGetAttributeValue("style:name") = xXMLStyleName then
                    SAXSeekElement("style:properties")
                    for i = 1 to 2
                        printlog " +- " & AttributeSearch(i)
                        if SAXGetAttributeValue(AttributeSearch(i)) <> AttributeValue(i) then
                            warnlog "Paragraph style: " & AttributeSearch(i) & " is not " & AttributeValue(i) & "!"
                        end if
                    next i
                    c=a          
                end if
                SAXSeekElement(0)         
            next c
            SAXSeekElement("/")
            printlog "    * * *"   
            '-------------------------------------------------------------------------
            printlog "----------"
            printlog "Textstyles"
            printlog "----------"
            printlog "++ Feature(s) to be searched for:"
            printlog " |"     
            SAXSeekElement("office:document-content")
            SAXSeekElement("office:body")
            SAXSeekElement("text:p", 12)
            SAXSeekElement("text:span")
            xXMLStyleName = SAXGetAttributeValue("text:style-name")
            printlog "text:style-name: " & xXMLStyleName 
            '/// <u>Text Style T8</u>
            '///+ Searching for: style:text-outline=&quot;true&quot;
            '///+ Searching for: style:text-crossing-out=&quot;single-line&quot;
            '///+ Searching for: fo:font-size=&quot;22pt&quot;
            '///+ Searching for: fo:text-shadow=&quot;1pt 1pt&quot;
            '///+ Searching for: style:text-underline=&quot;single&quot;
            '///+ Searching for: style:text-underline-color=&quot;font-color&quot; ///
            AttributeSearch(1) = "style:text-outline"
            AttributeValue(1) = "true"
      
            AttributeSearch(2) = "style:text-crossing-out"
            AttributeValue(2) = "single-line"
      
            AttributeSearch(3) = "fo:font-size"
            AttributeValue(3) = "22pt"
      
            AttributeSearch(4) = "fo:text-shadow"
            AttributeValue(4) = "1pt 1pt"
      
            AttributeSearch(5) = "style:text-underline"
            AttributeValue(5) = "single"
      
            AttributeSearch(6) = "style:text-underline-color"
            AttributeValue(6) = "font-color"

            SAXSeekElement("/")
            SAXSeekElement("office:document-content")
            SAXSeekElement("office:automatic-styles")
            a = SAXGetChildCount
            for c = 1 to a
                SAXSeekElement(c)
                if SAXGetAttributeValue("style:name") = xXMLStyleName then
                    SAXSeekElement("style:properties")
                    for g = 1 to 6
                        printlog " +- " & AttributeSearch(g)
                        if SAXGetAttributeValue(AttributeSearch(g)) <> AttributeValue(g) then
                            warnlog "Textstyle: " & AttributeSearch(g) & " is not " & AttributeValue(g) & "!"
                        end if
                    next g
                    c=a           
                end if
                SAXSeekElement(0)         
            next c
            printlog "    * * *"
            SAXSeekElement("/")
            '-------------------------------------------------------------------------     
            printlog "++ Feature(s) to be searched for:"
            printlog " |"     
            for g = 1 to 3
                SAXSeekElement("office:document-content")
                SAXSeekElement("office:body")
                SAXSeekElement("text:p" , 8)     
                SAXSeekElement(g)
                xXMLStyleName = SAXGetAttributeValue("text:style-name")
                printlog "text:style-name: " & xXMLStyleName
                '/// <u>Text Style T5</u>
                '///+ Searching for: fo:font-style=&quot;italic&quot; ///
                '/// <u>Text Style T6</u>
                '///+ Searching for: fo:font-weight=&quot;bold&quot; ///
                '/// <u>Text Style T7</u>
                '///+ Searching for: style:text-underline=&quot;single&quot;
                '///+ Searching for: style:text-underline-color=&quot;font-color&quot; ///

                AttributeSearch(1) = "fo:font-style"
                AttributeValue(1) = "italic"
                
                AttributeSearch(2) = "fo:font-weight"
                AttributeValue(2) = "bold"
                
                AttributeSearch(3) = "style:text-underline"
                AttributeValue(3) = "single"
                
                AttributeSearch(4) = "style:text-underline-color"
                AttributeValue(4) = "font-color"
                
                SAXSeekElement("/")
                SAXSeekElement("office:document-content")
                SAXSeekElement("office:automatic-styles")
                a = SAXGetChildCount
                for c = 1 to a
                    SAXSeekElement(c)
                    if SAXGetAttributeValue("style:name") = xXMLStyleName then
                        SAXSeekElement("style:properties")
                        if g = 3 then 
                            for i = 1 to 2
                                printlog " +- " & AttributeSearch(i+2)
                                if SAXGetAttributeValue(AttributeSearch(i+2)) <> AttributeValue(i+2) then
                                    warnlog "Textstyle: " & AttributeSearch(i+2) & " is not " & AttributeValue(i+2) & "!"
                                end if
                            next i
                        else
                            printlog " +- " & AttributeSearch(g)
                            if SAXGetAttributeValue(AttributeSearch(g)) <> AttributeValue(g) then
                                warnlog "Textstyle: " & AttributeSearch(g) & " is not " & AttributeValue(g) & "!"
                            end if          
                        end if
                        c=a          
                    end if
                    SAXSeekElement(0)        
                next c
                SAXSeekElement("/")
            next g           
            printlog "    * * *"
            SAXSeekElement("/")
            '-------------------------------------------------------------------------
            printlog "++ Feature(s) to be searched for:"
            printlog " |"     
            SAXSeekElement("office:document-content")
            SAXSeekElement("office:body")
            SAXSeekElement("text:p", 19)
            xXMLStyleName = SAXGetAttributeValue("text:style-name")
            printlog "paragraph:style-name: " & xXMLStyleName 
            '/// <u>Paragraph Style P3</u>
            '///+ Searching in language 01 for : fo:letter-spacing=&quot;0.06cm&quot;
            '///+ Searching in other language for: fo:letter-spacing=&quot;
            '///+ Searching for: style:letter-kerning=&quot;true&quot;
            '-------------------------------------------------------------------------
            AttributeSearch(1) = "style:letter-kerning"
            AttributeValue(1) = "true"

            AttributeSearch(2) = "fo:letter-spacing"
            AttributeValue(2) = "0.106cm"    
            
            SAXSeekElement("/")
            SAXSeekElement("office:document-content")
            SAXSeekElement("office:automatic-styles")
            
            a = SAXGetChildCount     
            for c = 1 to a
                SAXSeekElement(c)
                if SAXGetAttributeValue("style:name") = xXMLStyleName then
                    SAXSeekElement("style:properties")
                    if gLanguage = 1 then 
                        b = 2
                    else
                        b = 1
                    end if
                    for g = 1 to b
                        printlog " +- " & AttributeSearch(g)
                        if SAXGetAttributeValue(AttributeSearch(g)) <> AttributeValue(g) then
                            warnlog "Textstyle: " & AttributeSearch(g) & " is not " & AttributeValue(g) & "!"
                        end if
                    next g
                    c=a           
                end if
                SAXSeekElement(0)         
            next c    
            '-------------------------------------------------------------------------
            printlog "    * * *"
            SAXSeekElement("/")
            '-------------------------------------------------------------------------
            printlog "++ Feature(s) to be searched for:"
            printlog " |" 
            for g = 1 to 2
                SAXSeekElement("office:document-content")
                SAXSeekElement("office:body")
                SAXSeekElement("text:p", 26)
                SAXSeekElement(g)
                xXMLStyleName = SAXGetAttributeValue("text:style-name")
                printlog "text:style-name: " & xXMLStyleName 
                '/// <u>Text Style T10</u>
                '///+ Searching for: style:text-background-color=&quot;#00dcff&quot; ///
                '/// <u>Text Style T11</u>
                '///+ Searching for: style:text-background-color=&quot;#ffcc99&quot; ///
                '-------------------------------------------------------------------------
                AttributeSearch(1) = "style:text-background-color"
                AttributeValue(1) = "#00dcff"

                AttributeSearch(2) = "style:text-background-color"
                AttributeValue(2) = "#ffcc99"
                
                SAXSeekElement("/")
                SAXSeekElement("office:document-content")
                SAXSeekElement("office:automatic-styles")
                
                a = SAXGetChildCount
                for c = 1 to a
                    SAXSeekElement(c)
                    if SAXGetAttributeValue("style:name") = xXMLStyleName then
                        SAXSeekElement("style:properties")
                        printlog " +- " & AttributeSearch(g)
                        if SAXGetAttributeValue(AttributeSearch(g)) <> AttributeValue(g) then
                            warnlog "Textstyle: " & AttributeSearch(g) & " is not " & AttributeValue(g) & "!"
                        end if          
                        c=a          
                    end if
                    SAXSeekElement(0)        
                next c
                SAXSeekElement("/")
            next g
            '-------------------------------------------------------------------------
            printlog "---------"
            printlog "O t h e r" 
            printlog "---------"
            '/// <u>Other (Link)</u>
            '///+ Searching for: &lt;text:a xlink:type=&quot;simple&quot; xlink:href=&quot;http://www.hyperlink.de/&quot; office:target-frame-name=&quot;_blank&quot; xlink:show=&quot;new&quot;&gt;Hyperlink&lt;/text:a&gt; ///
            printlog "    * * *"
            SAXSeekElement("/")
            '-------------------------------------------------------------------------
            printlog "++ Feature(s) to be searched for:"
            printlog " |"     
            SAXSeekElement("office:document-content")
            SAXSeekElement("office:body")
            SAXSeekElement("text:p", 29)
            SAXSeekElement("text:a")
            xXMLStyleName = SAXGetAttributeValue("text:style-name")
            printlog "text:style-name: " & xXMLStyleName 
     
            AttributeSearch(1) = "xlink:type"
            AttributeValue(1)  = "simple"
            
            AttributeSearch(2) = "xlink:href"
            AttributeValue(2)  = "http://www.hyperlink.de/"
            
            AttributeSearch(3) = "office:target-frame-name"
            AttributeValue(3)  = "_blank"
            
            AttributeSearch(4) = "xlink:show"
            AttributeValue(4)  = "new"
            
            for i = 1 to 4
                if SAXGetAttributeValue(AttributeSearch(i)) <> AttributeValue(i) then
                    if i = 3 then
                        warnlog "#i80480# Hyperlink attribute office:target-frame-name gets lost sometimes."
                    elseif i = 4 then
                        warnlog "#i80480# Hyperlink attribute xlink:show gets lost sometimes."
                    else
                        warnlog "Other: " & AttributeSearch(i) & " is not " & AttributeValue(i) & "!"
                    endif
                end if          
            next i
        end if
        SAXRelease()
    else
        call hCloseDocument
    end if
endcase

