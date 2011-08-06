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
'* owner : oliver.craemer@oracle.com
'*
'* short description : XML Calc Include File
'*
'\***********************************************************************************
   Dim Isliste(250) as string
   Dim OutputPath as string

sub sxc7_01

   printlog "--------------------------- sxc_01.inc ------------------------------"
   printlog "------------------------ Alien Attributes ---------------------------"
   call tAlienAttributes
   printlog "---------------------- File Format Changes --------------------------"
   call tfeat906
   printlog "------------------------------ Cells --------------------------------"
   call tCellformats1

   call tCellformats2
   call tCellformats3

end sub

'-------------------------------------------------------------------------
testcase tAlienAttributes
    Dim i as integer
    Dim a as integer
    Dim c as integer
    Dim xXMLStyleName as string
    Dim xPath as string
    Dim AttrNameInDOM as string
    Dim AttributeSearch(50) as string
    Dim AttributeValue(50) as string
    
    printlog "+- alien_attributes.sxc"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\calc\ooo10\alien_attributes.sxc"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\calc\level1\alien_attributes.sxc") , "StarOffice XML (Calc)") then
        '/// Closing the document also if there is a verification dialog.
        call hCloseDocument()
        sleep(3)     
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\calc\level1\alien_attributes.sxc") , gOfficePath & ConvertPath("user\work\xml\calc\level1\alien_attributes") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\calc\level1\alien_attributes\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\calc\level1\alien_attributes\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\calc\level1\alien_attributes\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\calc\level1\alien_attributes.sxc"))
            sleep(2)
            call hCloseDocument
            sleep(2)
            SAXReadFile(gOfficePath & ConvertPath("user\work\xml\calc\level1\alien_attributes\content.xml"))
            printlog "-----------------"
            printlog "Hidden Namespaces"
            printlog "-----------------"
            '/// <u><b>content.xml</u></b>
            '/// <u>Hidden attributes in namespaces</u>
            '///+ Searching for: xmlns:foo=&quot;http://openoffice.org/2000/foo&quot;
            '///+ Searching for: xmlns:bla=&quot;http://openoffice.org/2000/bla&quot;
            '-------------------------------------------------------------------------
            AttributeSearch(1) = "xmlns:foo"
            AttributeValue(1) = "http://openoffice.org/2000/foo"

            AttributeSearch(2) = "xmlns:bla"
            AttributeValue(2) = "http://openoffice.org/2000/bla"
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
            printlog "    * * *"
            '/// <u>hidden attributes</u>
            '///+<ol><li><b>in a cell style</b></li>
            '///+<li><b>in a paragraph style (twice)</b></li>
            '///+<li><b>in a text style (twice)</b></li>
            '///+<li><b>in a graphic style</b></li></ol>
            '///+ Searching for: bla=&quot;holla&quot;
            '///+ Searching for: bla1=&quot;holla1&quot;
            '///+ Searching for: bla2=&quot;holla2&quot;
            '///+ Searching for: foo:foobla=&quot;holla&quot;
            '///+ Searching for: foo:foobla1=&quot;holla1&quot;
            '///+ Searching for: foo:foobla2=&quot;holla2&quot;
            '///+ Searching for: bla:blabla=&quot;holla&quot;
            '///+ Searching for: bla:blabla1=&quot;holla1&quot;
            '///+ Searching for: bla:blabla2=&quot;holla2&quot;
            AttributeSearch(1) = "bla"
            AttributeValue(1) = "holla"
            AttributeSearch(2) = "bla1"
            AttributeValue(2) = "holla1"
            AttributeSearch(3) = "bla2"
            AttributeValue(3) = "holla2"
            AttributeSearch(4) = "foo:foobla"
            AttributeValue(4) = "holla"
            AttributeSearch(5) = "foo:foobla1"
            AttributeValue(5) = "holla1"
            AttributeSearch(6) = "foo:foobla2"
            AttributeValue(6) = "holla2"
            AttributeSearch(7) = "bla:blabla"
            AttributeValue(7) = "holla"
            AttributeSearch(8) = "bla:blabla1"
            AttributeValue(8) = "holla1"
            AttributeSearch(9) = "bla:blabla2"
            AttributeValue(9) = "holla2"
            '-------------------------------------------------------------------------
            printlog "++ Feature(s) to be searched for:"
            printlog " |"
            '-------------------------------------------------------------------------
            for c = 1 to 6
                if c = 1 then
                    printlog "---------------------------------------"
                    printlog "Hidden attributes in a cell style"
                    printlog "---------------------------------------"
                end if
                if c = 2 OR c = 3 then
                    printlog "---------------------------------------"
                    printlog "Hidden attributes in a paragraph style (" & c-1 & ")"
                    printlog "---------------------------------------"
                end if
                if c = 4 or c = 5 then
                    printlog "---------------------------------------"
                    printlog "Hidden attributes in a text style (" & c-3 & ")"
                    printlog "---------------------------------------"
                end if
                if c = 6 then
                    printlog "---------------------------------------"
                    printlog "Hidden attributes in a graphic style"
                    printlog "---------------------------------------"
                end if
                SAXSeekElement("/")
                SAXSeekElement("office:document-content")
                SAXSeekElement("office:automatic-styles")
                'The hidden attributes are in a known sequence included,
                'so the same routine will be used for different styles!
      
                'Note: The first four style:style elements do not have any hidden attributes -> 4+c.
                SAXSeekElement("style:style" , 4+c )
                AttrNameInDOM = SAXGetAttributeValue("style:name")
                printlog " + Style name: " & AttrNameInDOM
                printlog " |"
                SAXSeekElement("style:properties")
                for i = 1 to 9
                    printlog " +- " & AttributeSearch(i)
                    if SAXGetAttributeValue(AttributeSearch(i)) <> AttributeValue(i) then
                        'c = <nr> means <nr>'th+4 style:style in DOM tree
                        if c = 1 then 
                            warnlog "Hidden attributes (in a cell style): " & AttributeSearch(i) & " is not " & AttributeValue(i) & "!"
                        end if
                        if c = 2 or c = 3 then 
'                            warnlog "Hidden attributes (in a paragraph style): " & AttributeSearch(i) & " is not " & AttributeValue(i) & "!"
							warnlog "#i100879# : Hidden attributes (in a paragraph style): " & AttributeSearch(i) & " is not " & AttributeValue(i) & "!"
                        end if
                        if c = 4 or c = 5 then 
                            warnlog "Hidden attributes (in a text style): " & AttributeSearch(i) & " is not " & AttributeValue(i) & "!"
                        end if
                        if c = 6 then 
                            warnlog "Hidden attributes (in a graphic style): " & AttributeSearch(i) & " is not " & AttributeValue(i) & "!"
                        end if
                    end if
                next i
            next c
            SAXSeekElement("/")
            printlog "    * * *"
            '-------------------------------------------------------------------------
            SAXRelease()
            SAXReadFile(gOfficePath & ConvertPath("user\work\xml\calc\level1\alien_attributes\styles.xml"))
            '-------------------------------------------------------------------------
            printlog "    -------------------"
            printlog "    s t y l e s . x m l"
            printlog "    -------------------"
            printlog "-----------------"
            printlog "Hidden Namespaces"
            printlog "-----------------"
            '/// <u><b>styles.xml</u></b>
            '/// <u>Hidden attributes in namespaces</u>
            '///+ Searching for: xmlns:foo=&quot;http://openoffice.org/2000/foo&quot;
            '///+ Searching for: xmlns:bla=&quot;http://openoffice.org/2000/bla&quot;
            '-------------------------------------------------------------------------
            AttributeSearch(1) = "xmlns:foo"
            AttributeValue(1) = "http://openoffice.org/2000/foo"

            AttributeSearch(2) = "xmlns:bla"
            AttributeValue(2) = "http://openoffice.org/2000/bla"
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
            printlog "    * * *"
            SAXSeekElement("/")
            '-------------------------------------------------------------------------
            '/// <u>hidden attributes</u>
            '///+<ol><li><b>in a table cell template style</b></li>
            '///+<li><b>in a master page template style</b></li></ol>
            '///+ Searching for: bla=&quot;holla&quot;
            '///+ Searching for: bla1=&quot;holla1&quot;
            '///+ Searching for: bla2=&quot;holla2&quot;
            '///+ Searching for: foo:foobla=&quot;holla&quot;
            '///+ Searching for: foo:foobla1=&quot;holla1&quot;
            '///+ Searching for: foo:foobla2=&quot;holla2&quot;
            '///+ Searching for: bla:blabla=&quot;holla&quot;
            '///+ Searching for: bla:blabla1=&quot;holla1&quot;
            '///+ Searching for: bla:blabla2=&quot;holla2&quot;
            AttributeSearch(1) = "bla"
            AttributeValue(1) = "holla"
            AttributeSearch(2) = "bla1"
            AttributeValue(2) = "holla1"
            AttributeSearch(3) = "bla2"
            AttributeValue(3) = "holla2"
            AttributeSearch(4) = "foo:foobla"
            AttributeValue(4) = "holla"
            AttributeSearch(5) = "foo:foobla1"
            AttributeValue(5) = "holla1"
            AttributeSearch(6) = "foo:foobla2"
            AttributeValue(6) = "holla2"
            AttributeSearch(7) = "bla:blabla"
            AttributeValue(7) = "holla"
            AttributeSearch(8) = "bla:blabla1"
            AttributeValue(8) = "holla1"
            AttributeSearch(9) = "bla:blabla2"
            AttributeValue(9) = "holla2"
            '-------------------------------------------------------------------------
            printlog "++ Feature(s) to be searched for:"
            printlog " |"
            '-------------------------------------------------------------------------
            for c = 1 to 2
                SAXSeekElement("/")
                if c = 1 then
                    printlog "-----------------------------------------------"
                    printlog "Hidden attributes in a template of a cell style"
                    printlog "-----------------------------------------------"
                    SAXSeekElement("office:document-styles")
                    SAXSeekElement("office:styles")
                    SAXSeekElement("style:style" , 2)
                    AttrNameInDOM = SAXGetAttributeValue("style:name")
                    printlog " + Style name: " & AttrNameInDOM
                    printlog " |"
                end if
                if c = 2 then
                    printlog "------------------------------------------------------"
                    printlog "Hidden attributes in a template of a master page style"
                    printlog "------------------------------------------------------"
                    SAXSeekElement("office:document-styles")
                    SAXSeekElement("office:automatic-styles")
                    SAXSeekElement("style:page-master")
                    AttrNameInDOM = SAXGetAttributeValue("style:name")
                    printlog " + Style name: " & AttrNameInDOM
                    printlog " |"
                end if
                SAXSeekElement("style:properties")
                for i = 1 to 9
                    printlog " +- " & AttributeSearch(i)
                    if SAXGetAttributeValue(AttributeSearch(i)) <> AttributeValue(i) then
                        if c = 1 then 
                            warnlog "Hidden attributes (in a template of a cell style): " & AttributeSearch(i) & " is not " & AttributeValue(i) & "!"
                        end if
                        if c = 2 then 
                            warnlog "Hidden attributes (in a template of a master page style): " & AttributeSearch(i) & " is not " & AttributeValue(i) & "!"
                        end if
                    end if
                next i
            next c
            SAXSeekElement("/")
            '-------------------------------------------------------------------------
            printlog "    * * *"
            SAXRelease()
        end if
    else
        call hCloseDocument
    end if
endcase

'-------------------------------------------------------------------------

testcase tfeat906
    Dim i as integer
    Dim a as integer
    Dim c as integer
    Dim xXMLStyleName as string
    Dim xPath as string
    Dim AttrNameInDOM as string
    Dim AttributeSearch(50) as string
    Dim AttributeValue(50) as string

    printlog "+- feat906.sxc"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\calc\ooo10\feat906.sxc"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\calc\level1\feat906.sxc") , "StarOffice XML (Calc)") then
        '/// Closing the document also if there is a verification dialog.
        call hCloseDocument()
        sleep(3)     
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\calc\level1\feat906.sxc") , gOfficePath & ConvertPath("user\work\xml\calc\level1\feat906") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\calc\level1\feat906\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\calc\level1\feat906\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\calc\level1\feat906\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\calc\level1\feat906.sxc"))
            sleep(2)
            call hCloseDocument
            sleep(2)
            SAXReadFile(gOfficePath & ConvertPath("user\work\xml\calc\level1\feat906\content.xml"))
            printlog "-------------------------------"
            printlog "draw:notify-on-update-of-ranges"
            printlog "-------------------------------"
            '/// <u><b>content.xml</u></b>
            '/// <u>draw:notify-on-update-of-ranges</u>
            '/// <i>Feature (Change) 906</i>
            '///+  <blockquote>The draw:object got a new attribute. This attribute is only
            '///+ on chart objects given. It gives the ranges or tablenames
            '///+ of the chart (the ranges or tablenames the chart is
            '///+ connected to). If the attribute is not given the object
            '///+ will be loaded otherwise the object is only loaded if in
            '///+ the ranges or tables is something changed or the object is
            '///+ showed. If the value of the attribute is empty it is a OLE
            '///+ chart with its own data.
            '///+ This increase the performance of loading charts, because
            '///+ they are only loaded if they are needed.<br>
            '///+ This attribute <u>replaces</u> the <i>draw:notify-on-update-table</i>
            '///+ attribute which only was usable by the wordprocessing
            '///+ application. The new one is used by the wordprocessing and
            '///+ the spreadsheet application.
            '///+ The data in this attribute are twice, because the chart has
            '///+ this data too, but to get this data from the chart it has
            '///+ to be loaded.</blockquote>
            '/// Searching for: draw:notify-on-update-of-ranges=&quot;Sheet1.A1:Sheet1.A5&quot;
            
            '-------------------------------------------------------------------------
            AttributeSearch(1) = "draw:notify-on-update-of-ranges"
            AttributeValue(1) = "Sheet1.A1:Sheet1.A5"

            printlog "++ Feature(s) to be searched for:"
            printlog " |"
            '-------------------------------------------------------------------------
            SAXSeekElement("office:document-content")
            SAXSeekElement("office:body")
            SAXSeekElement("table:table")
            SAXSeekElement("table:table-row" , 2)
            SAXSeekElement("table:table-cell" , 2)
            SAXSeekElement("draw:object")
            for i = 1 to 1
                printlog " +- " & AttributeSearch(i)
                if SAXGetAttributeValue(AttributeSearch(i)) <> AttributeValue(i) then
                    warnlog "Hidden attributes (in namespace): " & AttributeSearch(i) & " is not " & AttributeValue(i) & "!"
                end if
            next i
            printlog "    * * *"
            SAXRelease()
        end if
    else
        call hCloseDocument
    end if
endcase

'-------------------------------------------------------------------------

testcase tCellformats1
    Dim i as integer
    Dim a as integer
    Dim xXMLStyleName as string
    Dim xPath as string
    Dim AttributeSearch(50) as string
    Dim AttributeValue(50) as string

    printlog "+- cellformats1.sdc"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\calc\so_binary\cellformats1.sdc"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\calc\level1\cellformats1.sxc") , "StarOffice XML (Calc)") then
        '/// Closing the document also if there is a verification dialog.
        call hCloseDocument()
        sleep(3)     
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\calc\level1\cellformats1.sxc") , gOfficePath & ConvertPath("user\work\xml\calc\level1\cellformats1") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\calc\level1\cellformats1\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\calc\level1\cellformats1\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\calc\level1\cellformats1\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\calc\level1\cellformats1.sxc"))
            sleep(2)
            call hCloseDocument
            sleep(2)
            printlog "    ---------------------"
            printlog "    c o n t e n t . x m l"
            printlog "    ---------------------"

            '/// <u><b>content.xml</u></b>
            SAXReadFile(gOfficePath & ConvertPath("user\work\xml\calc\level1\cellformats1\content.xml"))
            printlog "-----------"
            printlog "T A B L E S"
            printlog "-----------"
            '/// <u>table style t1</u>
            '///+ Searching for: style:master-page-name=&quot;Default&quot; ///
            '///+ Searching for: style:family=&quot;table&quot; ///
            '-------------------------------------------------------------------------
            AttributeSearch(1) = "style:family"
            AttributeValue(1) = "table"

            AttributeSearch(2) = "style:master-page-name"
            AttributeValue(2) = "Default"
            printlog "++ Feature(s) to be searched for:"
            printlog " |"
            '-------------------------------------------------------------------------
            xXMLStyleName = GetItemStyleName ( "office:body", "table:table", "table:table" , 1 , "table:style-name" )
            printlog " +---------------------------------------------------------"
            printlog " |" & xXMLStyleName
            printlog " +---------------------------------------------------------"
            xPath = GetXMLElementPath( "office:automatic-styles" , "style:style" , "style:name" , xXMLStyleName)
            for i = 1 to 2
                SAXSeekElement(xPath)
                printlog " +- " & AttributeSearch(i)
                if SAXGetAttributeValue(AttributeSearch(i)) <> AttributeValue(i) then
                    warnlog "Table Style: " & AttributeSearch(i) & " is not " & AttributeValue(i) & "!"
                end if
            next i
            '///+ &nbsp;<i>style properties:</i>
            '///+ &nbsp;Searching for: table:display=&quot;true&quot;
            '-------------------------------------------------------------------------
            AttributeSearch(1) = "table:display"
            AttributeValue(1) = "true"
            '-------------------------------------------------------------------------
            SAXSeekElement(xPath)
            SAXSeekElement("style:properties")
            printlog " +- " & AttributeSearch(1)
            if SAXGetAttributeValue(AttributeSearch(1)) <> AttributeValue(1) then
                warnlog "style:properties: " & AttributeSearch(1) & " is not " & AttributeValue(1) & "!"
            end if
            SAXSeekElement("/")
            printlog "    * * *"

            printlog "---------"
            printlog "C E L L S"
            printlog "---------"

            '/// <u>cell style ce2</u>
            '///+ Searching for: style:family=&quot;table-cell&quot;
            '///+ Searching for: style:parent-style-name=&quot;Default&quot;
            '-------------------------------------------------------------------------
            AttributeSearch(1) = "style:family"
            AttributeValue(1) = "table-cell"
            AttributeSearch(2) = "style:parent-style-name"
            AttributeValue(2) = "Default"
            printlog "++ Feature(s) to be searched for:"
            printlog " |"
            '-------------------------------------------------------------------------
            SAXSeekElement("office:document-content")
            SAXSeekElement("office:body")
            SAXSeekElement("table:table")
            SAXSeekElement("table:table-row" , 2)
            SAXSeekElement("table:table-cell" , 2)
            xXMLStyleName = SAXGetAttributeValue("table:style-name")
            printlog " +---------------------------------------------------------"
            printlog " |" & xXMLStyleName
            printlog " +---------------------------------------------------------"
            xPath = GetXMLElementPath( "office:automatic-styles" , "style:style" , "style:name" , xXMLStyleName)
            for i = 1 to 2
                SAXSeekElement(xPath)
                printlog " +- " & AttributeSearch(i)
                if SAXGetAttributeValue(AttributeSearch(i)) <> AttributeValue(i) then
                    warnlog "Table Style: " & AttributeSearch(i) & " is not " & AttributeValue(i) & "!"
                end if
            next i
            '///+ &nbsp;<i>style properties:</i>
            '///+ Searching for: fo:color=&quot;#000000&quot;
            '///+ Searching for: style:font-name=&quot;Arial&quot;
            '///+ Searching for: fo:font-size=&quot;10pt&quot;
            '///+ Searching for: fo:font-style=&quot;italic&quot;
            '///+ Searching for: style:text-underline=&quot;single&quot;
            '///+ Searching for: fo:font-weight=&quot;bold&quot;
            '-------------------------------------------------------------------------
            AttributeSearch(1) = "fo:color"
            AttributeValue(1) = "#000000"
            AttributeSearch(2) = "style:font-name"
            AttributeValue(2) = "Arial"
            AttributeSearch(3)  = "fo:font-size"
            AttributeValue(3) = "10pt"
            AttributeSearch(4)  = "fo:font-style"
            AttributeValue(4) = "italic"
            AttributeSearch(5) = "style:text-underline"
            AttributeValue(5) = "single"
            AttributeSearch(6) = "fo:font-weight"
            AttributeValue(6) = "bold"
            '-------------------------------------------------------------------------
            SAXSeekElement(xPath)
            SAXSeekElement("style:properties")

            for i = 1 to  6
                printlog " +- " & AttributeSearch(i)
                if SAXGetAttributeValue(AttributeSearch(i)) <> AttributeValue(i) then
                    warnlog "style:properties: " & AttributeSearch(i) & " is not " & AttributeValue(i) & "!"
                end if
            next i
            SAXSeekElement("/")
            printlog "    * * *"

            '/// <u>cell 3</u>
            '///+ Searching for: style:family=&quot;table-cell&quot;
            '///+ Searching for: style:parent-style-name=&quot;Default&quot;
            '-------------------------------------------------------------------------
            AttributeSearch(1) = "style:family"
            AttributeValue(1) = "table-cell"
            AttributeSearch(2) = "style:parent-style-name"
            AttributeValue(2) = "Default"
            printlog "++ Feature(s) to be searched for:"
            printlog " |"
            '-------------------------------------------------------------------------
            SAXSeekElement(1)
            SAXSeekElement("office:body")
            SAXSeekElement("table:table")
            SAXSeekElement("table:table-row" , 4)
            SAXSeekElement("table:table-cell" , 2)
            xXMLStyleName = SAXGetAttributeValue("table:style-name")
            printlog " +---------------------------------------------------------"
            printlog " |" & xXMLStyleName
            printlog " +---------------------------------------------------------"
            xPath = GetXMLElementPath( "office:automatic-styles" , "style:style" , "style:name" , xXMLStyleName)
            for i = 1 to 2
                SAXSeekElement(xPath)
                printlog " +- " & AttributeSearch(i)
                if SAXGetAttributeValue(AttributeSearch(i)) <> AttributeValue(i) then
                    warnlog "Table Style: " & AttributeSearch(i) & " is not " & AttributeValue(i) & "!"
                end if
            next i
            '///+ &nbsp;<i>style properties:</i>
            '///+ Searching for: fo:color=&quot;#800000&quot;
            '///+ Searching for: style:text-outline=&quot;true&quot;
            '///+ Searching for: style:text-crossing-out=&quot;single-line&quot;
            '///+ Searching for: fo:font-size=&quot;12pt&quot;
            '///+ Searching for: fo:language=&quot;en&quot;
            '///+ Searching for: fo:country=&quot;US&quot;
            '///+ Searching for: fo:text-shadow=&quot;none&quot;
            '-------------------------------------------------------------------------
            AttributeSearch(1) = "fo:color"
            AttributeValue(1) = "#800000"
            AttributeSearch(2) = "style:text-outline"
            AttributeValue(2) = "true"
            AttributeSearch(3)  = "fo:font-size"
            AttributeValue(3) = "12pt"
            AttributeSearch(4)  = "style:text-crossing-out"
            AttributeValue(4) = "single-line"
            AttributeSearch(5) = "fo:text-shadow"
            AttributeValue(5) = "none"
            AttributeSearch(6) = "fo:country"
            AttributeValue(6) = "US"
            AttributeSearch(7) = "fo:language"
            AttributeValue(7) = "en"
            '-------------------------------------------------------------------------
            SAXSeekElement(xPath)
            SAXSeekElement("style:properties")

            for i = 1 to  7
                printlog " +- " & AttributeSearch(i)
                if SAXGetAttributeValue(AttributeSearch(i)) <> AttributeValue(i) then
                    warnlog "style:properties: " & AttributeSearch(i) & " is not " & AttributeValue(i) & "!"
                end if
            next i
            SAXSeekElement("/")
            printlog "    * * *"

            '/// <u>cell 4</u>
            '///+ &nbsp;<i>style properties:</i>
            '///+ Searching for: style:font-name=&quot;Times New Roman&quot;
            '///+ Searching for: fo:font-size=&quot;28pt&quot;
            '-------------------------------------------------------------------------
            AttributeSearch(1) = "style:font-name"
            AttributeValue(1) = "Comic Sans MS"
            AttributeSearch(2) = "fo:font-size"
            AttributeValue(2) = "16pt"
            '-------------------------------------------------------------------------
            SAXSeekElement(1)
            SAXSeekElement("office:body")
            SAXSeekElement("table:table")
            SAXSeekElement("table:table-row", 6)
            SAXSeekElement("table:table-cell" , 2)
            xXMLStyleName = SAXGetAttributeValue("table:style-name")
            printlog " +---------------------------------------------------------"
            printlog " |" & xXMLStyleName
            printlog " +---------------------------------------------------------"
            xPath = GetXMLElementPath( "office:automatic-styles" , "style:style" , "style:name" , xXMLStyleName)
            SAXSeekElement(xPath)
            SAXSeekElement(1)
            for i = 1 to 2
                printlog " +- " & AttributeSearch(i)
                if SAXGetAttributeValue(AttributeSearch(i)) <> AttributeValue(i) then
                    warnlog "style:properties: " & AttributeSearch(i) & " is not " & AttributeValue(i) & "!"
                end if
            next i
            SAXRelease()
        end if
    else
        call hCloseDocument
    end if
endcase
'
'-------------------------------------------------------------------------
'
testcase tCellformats2
    printlog "+- cellformats2.sdc"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\calc\so_binary\cellformats2.sdc"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\calc\level1\cellformats2.sxc") , "StarOffice XML (Calc)") then
        '/// Closing the document also if there is a verification dialog.
        call hCloseDocument()
        sleep(3)     
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\calc\level1\cellformats2.sxc") , gOfficePath & ConvertPath("user\work\xml\calc\level1\cellformats2") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\calc\level1\cellformats2\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\calc\level1\cellformats2\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\calc\level1\cellformats2\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\calc\level1\cellformats2.sxc"))
            sleep(2)
            call hCloseDocument
            sleep(2)
        end if
    else
        call hCloseDocument
    end if
endcase
'
'-------------------------------------------------------------------------
'
testcase tCellformats3
    printlog "+- cellformats3.sdc"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\calc\so_binary\cellformats3.sdc"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\calc\level1\cellformats3.sxc") , "StarOffice XML (Calc)") then
        '/// Closing the document also if there is a verification dialog.
        call hCloseDocument()
        sleep(3)     
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\calc\level1\cellformats3.sxc") , gOfficePath & ConvertPath("user\work\xml\calc\level1\cellformats3") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\calc\level1\cellformats3\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\calc\level1\cellformats3\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\calc\level1\cellformats3\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\calc\level1\cellformats3.sxc"))
            sleep(2)
            call hCloseDocument
            sleep(2)
        end if
    else
        call hCloseDocument
    end if
endcase

