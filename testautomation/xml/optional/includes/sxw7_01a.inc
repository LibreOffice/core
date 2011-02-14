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

sub sxw7_01a

   printlog "-------------------------- SXW7_01a.INC ------------------------------"

   printlog "---------------------------------------------------------------------"
   call tfields
   call tfootnotes

end sub

'-------------------------------------------------------------------------
   
testcase tfields
    Dim a as integer
    Dim i as integer
    Dim AttributeSearch(60) as string
    Dim AttributeValue(60) as string
    Dim sXMLNodename(50) as string
    Dim sPathToAttributes as string

    printlog "+- fields.sdw"
    '/// Opening file fields.sdw from input-directory.
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\writer\fields.sdw"))
    '/// Saving the document in OOo XML 1.0 format.
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\writer\level1\fields.sxw") , "StarOffice XML (Writer)") then    
        '/// Closing the document also if there is a verification dialog.    
        call hCloseDocument()    
        sleep(3)
        '/// Unpack the complete content of the file (ZIP-archieve) into a seperate directory.
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\writer\level1\fields.sxw") , gOfficePath & ConvertPath("user\work\xml\writer\level1\fields") )        
        '/// Checking the well formness of content.xml, styles.xml and meta.xml.
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\writer\level1\fields\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if   
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\writer\level1\fields\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\writer\level1\fields\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            '/// Opening the exported file (GPF check).
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\writer\level1\fields.sxw"))
            sleep(2)
            '/// Closing the document.
            call hCloseDocument
            sleep(2)
            SAXReadFile(gOfficePath & ConvertPath("user\work\xml\writer\level1\fields\content.xml"))
            printlog "    ---------------------"
            printlog "    c o n t e n t . x m l"
            printlog "    ---------------------"
            printlog "----------"
            printlog "  fields  "
            printlog "----------"    
            '-------------------------------------------------------------------------
            '/// <u><b>CONTENT.XML</u></b>
            printlog "++ Feature(s) to be searched for:"
            printlog " |"     
            '-------------------------------------------------------------------------
            '/// Searching for: &lt;text:sender-firstname text:fixed=&quot;false&quot;&gt; 
            '/// Searching for: &lt;text:sender-lastname text:fixed=&quot;false&quot;&gt;
            sXMLNodename(1) = "text:sender-firstname"
            sXMLNodename(2) = "text:sender-lastname"
            AttributeSearch(1) = "text:fixed"
            AttributeValue(1) = "false"    
            '/// Searching for: &lt;text:date text:date-value=&quot;2000-11-30T16:14:56&quot; 
            '/// Searching for: &lt;text:time text:time-value=&quot;[not null]&quot;  
            sXMLNodename(3) = "text:date"     
            AttributeSearch(3) = "text:date-value"
            AttributeValue(3) = "2000-11-30T16:14:56"
            sXMLNodename(4) = "text:time"
            AttributeSearch(4) = "text:time-value"
            AttributeValue(4) = ""     
            '/// Searching for: &lt;text:word-count style:num-format=&quot;1&quot;&gt;75&lt;/text:word-count&gt;   
            '/// Searching for: &lt;text:character-count style:num-format=&quot;1&quot;&gt;788&lt;/text:character-count&gt;
            sXMLNodename(5) = "text:word-count"
            'TODO: JSI: Rotine needed for char XML node
            'CharNode = 75
            sXMLNodename(6) = "text:character-count"
            'TODO: JSI: Rotine needed for char XML node
            'CharNode = 788    
            AttributeSearch(5) = "style:num-format" 
            AttributeValue(5) = "1"    
            '/// Searching for: &lt;text:reference-mark-start text:name=&quot;ref&quot;/&gt;I am a reference&lt;text:reference-mark-end text:name=&quot;ref&quot;/&gt;
            sXMLNodename(7) = "text:reference-mark-start"    
            sXMLNodename(8) = "text:reference-mark-end"
            AttributeSearch(7) = "text:name"
            AttributeValue(7) = "ref"
            '/// Searching for: &lt;text:reference-ref text:reference-format=&quot;page&quot; text:ref-name=&quot;ref&quot;&gt;1&lt;/text:reference-ref&gt; Chapter: &lt;text:reference-ref text:reference-format=&quot;chapter&quot; text:ref-name=&quot;ref&quot;&gt;1&lt;/text:reference-ref&gt; Location: &lt;text:reference-ref text:reference-format=&quot;direction&quot; text:ref-name=&quot;ref&quot;&gt;above&lt;/text:reference-ref&gt;
            sXMLNodename(9)  = "text:reference-ref"
            'TODO: JSI: Rotine needed for char XML node
            'CharNode: 1 [9]
            'CharNode: 1 [10]
            'CharNode: above [11]
            AttributeSearch(9) = "text:reference-format"
            AttributeValue(9) = "page"
            AttributeValue(10) = "chapter"    
            AttributeValue(11) = "direction"    
            'All three have this attribute and -value:
            AttributeSearch(12) = "text:ref-name"
            AttributeValue(12)  = "ref"    
            '/// Searching for: &lt;text:variable-set text:name=&quot;x&quot; text:value-type=&quot;float&quot; text:value=&quot;1&quot; style:data-style-name=&quot;N0&quot;&gt;1&lt;/text:variable-set&gt;    
            sXMLNodename(13) = "text:variable-set"
            AttributeSearch(13) = "text:name"
            AttributeValue(13)  = "x"
            AttributeSearch(14) = "text:value-type"
            AttributeValue(14) = "float"
            AttributeSearch(15) = "text:value"
            AttributeValue(15) = "1" 
            AttributeSearch(16) = "style:data-style-name" 
            AttributeValue(16) = "N0"
            'TODO: JSI: Rotine needed for char XML node
            'CharNode: 1
            '/// Searching for: &lt;text:user-field-get text:name=&quot;I_am_a_user_field&quot;&gt;Content of the user field&lt;/text:user-field-get&gt;
            sXMLNodename(17) = "text:user-field-get"
            AttributeSearch(17) = "text:name" 
            AttributeValue(17) = "I_am_a_user_field"
            'TODO: JSI: Rotine needed for char XML node
            'CharNode: Content of the user field
            '/// Searching for: &lt;text:conditional-text text:condition=&quot;x==1&quot; text:string-value-if-true=&quot;x = 1&quot; text:string-value-if-false=&quot;x &amp;lt;&amp;gt; 1&quot;&gt;x = 1&lt;/text:conditional-text&gt; 
            sXMLNodename(18) = "text:conditional-text" 
            AttributeSearch(18) = "text:condition"
            AttributeValue(18) = "x==1"
            AttributeSearch(19) = "text:string-value-if-true"
            AttributeValue(19) = "x = 1"
            AttributeSearch(20) = "text:string-value-if-false"
            AttributeValue(20) = "x <> 1"
            'TODO: JSI: Rotine needed for char XML node
            'CharNode: x = 1
            '/// Searching for: &lt;text:hidden-text text:condition=&quot;x==1&quot; text:string-value=&quot;x is not equal to 1&quot;/&gt;
            sXMLNodename(21) = "text:hidden-text"
            AttributeSearch(21) = "text:condition"
            AttributeValue(21) = "x==1"
            AttributeSearch(22) = "text:string-value"
            AttributeValue(22) = "x is not equal to 1"    
            '/// Searching for: &lt;text:hidden-paragraph text:condition=&quot;x==1&quot; text:is-hidden=&quot;true&quot;/&gt;I am hidden if x=1&lt;/text:p&gt;
            sXMLNodename(23) = "text:hidden-paragraph"
            AttributeSearch(23) = "text:condition"
            AttributeValue(23) = "x==1"
            AttributeSearch(24) = "text:is-hidden"
            AttributeValue(24) = "true"
            'TODO: JSI: Rotine needed for char XML node
            'CharNode: I am hidden if x=1     
            '/// Searching for: &lt;text:creation-date style:data-style-name=&quot;N36&quot;&gt;15/11/2000&lt;/text:creation-date&gt;
            sXMLNodename(25) = "text:creation-date"
            AttributeSearch(25) = "style:data-style-name"
            AttributeValue(25) = "N108"   
            '/// Searching for: &apos;&lt;text:editing-cycles&gt;0&lt;/text:editing-cycles&gt;
            'sXMLNodename(?) = text:editing-cycles
            'TODO: JSI: Rotine needed for char XML node
            'CharNode: 0   
            '/// Searching for: &lt;text:modification-date style:data-style-name=&quot;N81&quot;&gt;6. mars 2003&lt;/text:modification-date&gt;
            sXMLNodename(26) = "text:modification-date"
            AttributeSearch(26) = "style:data-style-name"
            AttributeValue(26) = "N76"
            'TODO: JSI: Rotine needed for char XML node
            'CharNode: 6. mars 2003
            '/// Searching for: &lt;text:database-display text:database-name=&quot;Bibliography&quot; text:table-name=&quot;biblio&quot; text:table-type=&quot;table&quot; text:column-name=&quot;Author&quot;&gt;&amp;lt;Author&amp;gt;&lt;/text:database-display&gt;
            sXMLNodename(27) = "text:database-display"
            AttributeSearch(27) = "text:database-name"
            AttributeValue(27) = "Bibliography"
            AttributeSearch(28) = "text:table-name"
            AttributeValue(28) = "biblio"
            AttributeSearch(29) = "text:table-type"
            AttributeValue(29) = "table"
            AttributeSearch(30) = "text:column-name"
            AttributeValue(30) = "Author"   
            'TODO: JSI: Rotine needed for char XML node
            'CharNode: <Author>   
            '/// Searching for: &lt;text:database-next text:database-name=&quot;Bibliography&quot; text:table-name=&quot;biblio&quot; text:table-type=&quot;table&quot; text:condition=&quot;TRUE&quot;/&gt;
            sXMLNodename(31) = "text:database-next"
            AttributeSearch(31) = "text:database-name"
            AttributeValue(31) = "Bibliography"
            AttributeSearch(32) = "text:table-name"
            AttributeValue(32) = "biblio"
            AttributeSearch(33) = "text:table-type"
            AttributeValue(33) = "table"
            AttributeSearch(34) = "text:condition"
            AttributeValue(34) = "TRUE" 
            '/// Searching for: &lt;text:database-display text:database-name=&quot;Bibliography&quot; text:table-name=&quot;biblio&quot; text:table-type=&quot;table&quot; text:column-name=&quot;Author&quot;&gt;&amp;lt;Author&amp;gt;&lt;/text:database-display&gt;
            sXMLNodename(35) = "text:database-display"
            AttributeSearch(35) = "text:database-name"
            AttributeValue(35) = "Bibliography"
            AttributeSearch(36) = "text:table-name"
            AttributeValue(36) = "biblio"
            AttributeSearch(37) = "text:table-type"
            AttributeValue(37) = "table"
            AttributeSearch(38) = "text:column-name"
            AttributeValue(38) = "Author" 
            'TODO: JSI: Rotine needed for char XML node
            'CharNode: <Author>     
            '/// Searching for: &lt;text:database-row-number text:database-name=&quot;Bibliography&quot; text:table-name=&quot;biblio&quot; text:table-type=&quot;table&quot; style:num-format=&quot;1&quot; text:value=&quot;3&quot;&gt;3&lt;/text:database-row-number&gt;
            sXMLNodename(39) = "text:database-row-number"
            AttributeSearch(39) = "text:database-name"
            AttributeValue(39) = "Bibliography"
            AttributeSearch(40) = "text:table-name"
            AttributeValue(40) = "biblio"
            AttributeSearch(41) = "text:table-type"
            AttributeValue(41) = "table"
            AttributeSearch(42) = "style:num-format"
            AttributeValue(42) = "1"
            AttributeSearch(43) = "text:value"
            AttributeValue(43) = "3"
            'TODO: JSI: Rotine needed for char XML node
            'CharNode: 3  
            '/// Searching for: &lt;text:database-name text:database-name=&quot;Bibliography&quot; text:table-name=&quot;biblio&quot; text:table-type=&quot;table&quot;&gt;Bibliography.biblio&lt;/text:database-name&gt;
            sXMLNodename(44) = "text:database-name"
            AttributeSearch(44) = "text:database-name"
            AttributeValue(44) = "Bibliography"
            AttributeSearch(45) = "text:table-name"
            AttributeValue(45) = "biblio"
            AttributeSearch(46) = "text:table-type"
            AttributeValue(46) = "table"
            'TODO: JSI: Rotine needed for char XML node
            'CharNode: Bibliography.biblio    
            for a = 1 to 46       
                select case a
                    case 1 , 2 :
                        ' Two nodes have the same attributes and -values as the first one          
                        sPathToAttributes = fWhereIsXMLElementInBody("office:document-content" , "office:body" , sXMLNodename(a))
                        SAXSeekElement(sPathToAttributes)
                        printlog " +- [Element: " & sXMLNodename(a) & "] Attribute: " & AttributeSearch(1)           
                        if SAXGetAttributeValue(AttributeSearch(1)) <> AttributeValue(1) then
                            warnlog "Field: " & AttributeSearch(1) & " is not " & AttributeValue(1) & "!"
                        end if         
                        SAXSeekElement("/")
                    case 5 , 6  :
                        ' Two nodes have the same attributes and -values as the first one
                        sPathToAttributes = fWhereIsXMLElementInBody("office:document-content" , "office:body" , sXMLNodename(a))
                        SAXSeekElement(sPathToAttributes)          
                        printlog " +- [Element: " & sXMLNodename(a) & "] Attribute: " & AttributeSearch(5)           
                        if SAXGetAttributeValue(AttributeSearch(5)) <> AttributeValue(5) then
                            warnlog "Field: " & AttributeSearch(5) & " is not " & AttributeValue(5) & "!"
                        end if         
                        SAXSeekElement("/")
                    case 7 , 8  : ' Two nodes have the same attributes and -values as the first one                               
                        sPathToAttributes = fWhereIsXMLElementInBody("office:document-content" , "office:body" , sXMLNodename(a))
                        SAXSeekElement(sPathToAttributes)            
                        printlog " +- [Element: " & sXMLNodename(a) & "] Attribute: " & AttributeSearch(7)           
                        if SAXGetAttributeValue(AttributeSearch(7)) <> AttributeValue(7) then
                            warnlog "Field: " & AttributeSearch(7) & " is not " & AttributeValue(7) & "!"
                        end if         
                        SAXSeekElement("/")
                    case 3, 4 :
                        ' The attribute is language dependent and should NOT be NULL
                        ' The if-then-else inquiry is another than in the other inquiries!
                        sPathToAttributes = fWhereIsXMLElementInBody("office:document-content" , "office:body" , sXMLNodename(a))
                        SAXSeekElement(sPathToAttributes)            
                        printlog " +- [Element: " & sXMLNodename(a) & "] Attribute: " & AttributeSearch(a)           
                        if SAXGetAttributeValue(AttributeSearch(a)) = AttributeValue(a) then
                            warnlog "Field: " & AttributeSearch(a) & " is null empty!"
                        end if         
                        SAXSeekElement("/")          
                    case 9 , 10, 11:
                        ' Three times the same element name but not the same XML node
                        sPathToAttributes = fWhereIsXMLElementInBody("office:document-content" , "office:body" , sXMLNodename(9) , a-8)                      
                        SAXSeekElement(sPathToAttributes)
                        printlog " +- [Element: " & sXMLNodename(9) & "] Attribute: " & AttributeSearch(9) & "=" & AttributeValue(a)           
                        if SAXGetAttributeValue(AttributeSearch(9)) <> AttributeValue(a) then
                            warnlog "Field: " & AttributeSearch(9) & " is not " & AttributeValue(a) & "!"
                        end if
                        SAXSeekElement("/")                    
                    case 12:                
                        for i = 1 to 3
                        sPathToAttributes = fWhereIsXMLElementInBody("office:document-content" , "office:body" , sXMLNodename(9) , i) 
                        SAXSeekElement(sPathToAttributes)                     
                        printlog " +- [Element: " & sXMLNodename(9) & " (" & i & ")] Attribute: " & AttributeSearch(12) & "=" & AttributeValue(12)
                        if SAXGetAttributeValue(AttributeSearch(12)) <> AttributeValue(12) then
                            warnlog "Field: " & AttributeSearch(12) & " is not " & AttributeValue(12) & "!"
                        end if                       
                        SAXSeekElement("/")
                        next i
                    case 21, 22 :
                        ' One node has two attributes          
                        sPathToAttributes = fWhereIsXMLElementInBody("office:document-content" , "office:body" , sXMLNodename(21))
                        SAXSeekElement(sPathToAttributes)                      
                        printlog " +- [Element: " & sXMLNodename(21) & "]" & AttributeSearch(a)           
                        if SAXGetAttributeValue(AttributeSearch(a)) <> AttributeValue(a) then
                            warnlog "Field: " & AttributeSearch(a) & " is not " & AttributeValue(a) & "!"
                        end if         
                        SAXSeekElement("/")
                    case 23, 24 :
                        ' One node has two attributes          
                        sPathToAttributes = fWhereIsXMLElementInBody("office:document-content" , "office:body" , sXMLNodename(23))
                        SAXSeekElement(sPathToAttributes)                      
                        printlog " +- [Element: " & sXMLNodename(23) & "]" & AttributeSearch(a)           
                        if SAXGetAttributeValue(AttributeSearch(a)) <> AttributeValue(a) then
                            warnlog "Field: " & AttributeSearch(a) & " is not " & AttributeValue(a) & "!"
                        end if         
                        SAXSeekElement("/")                       
                    case 18, 19, 20 :
                        ' One node has three attributes          
                        sPathToAttributes = fWhereIsXMLElementInBody("office:document-content" , "office:body" , sXMLNodename(18))
                        SAXSeekElement(sPathToAttributes)                      
                        printlog " +- [Element: " & sXMLNodename(18) & "]" & AttributeSearch(a)           
                        if SAXGetAttributeValue(AttributeSearch(a)) <> AttributeValue(a) then
                            warnlog "Field: " & AttributeSearch(a) & " is not " & AttributeValue(a) & "!"
                        end if         
                        SAXSeekElement("/")
                    case 44, 45, 46 :
                        ' One node has three attributes          
                        sPathToAttributes = fWhereIsXMLElementInBody("office:document-content" , "office:body" , sXMLNodename(44))
                        SAXSeekElement(sPathToAttributes)                      
                        printlog " +- [Element: " & sXMLNodename(44) & "]" & AttributeSearch(a)           
                        if SAXGetAttributeValue(AttributeSearch(a)) <> AttributeValue(a) then
                            warnlog "Field: " & AttributeSearch(a) & " is not " & AttributeValue(a) & "!"
                        end if         
                        SAXSeekElement("/")                       
                    case 13, 14, 15, 16  :
                        ' One node has four attributes          
                        sPathToAttributes = fWhereIsXMLElementInBody("office:document-content" , "office:body" , sXMLNodename(13))
                        SAXSeekElement(sPathToAttributes)                      
                        printlog " +- [Element: " & sXMLNodename(13) & "]" & AttributeSearch(a)           
                        if SAXGetAttributeValue(AttributeSearch(a)) <> AttributeValue(a) then
                            warnlog "Field: " & AttributeSearch(a) & " is not " & AttributeValue(a) & "!"
                        end if         
                        SAXSeekElement("/")                                 
                    case 27, 28, 29, 30 :
                        ' One node has four attributes          
                        sPathToAttributes = fWhereIsXMLElementInBody("office:document-content" , "office:body" , sXMLNodename(27))
                        SAXSeekElement(sPathToAttributes)                      
                        printlog " +- [Element: " & sXMLNodename(27) & "]" & AttributeSearch(a)           
                        if SAXGetAttributeValue(AttributeSearch(a)) <> AttributeValue(a) then
                            warnlog "Field: " & AttributeSearch(a) & " is not " & AttributeValue(a) & "!"
                        end if         
                        SAXSeekElement("/")                                                           
                    case 31, 32, 33, 34 :
                        ' One node has four attributes          
                        sPathToAttributes = fWhereIsXMLElementInBody("office:document-content" , "office:body" , sXMLNodename(31))
                        SAXSeekElement(sPathToAttributes)                      
                        printlog " +- [Element: " & sXMLNodename(31) & "]" & AttributeSearch(a)           
                        if SAXGetAttributeValue(AttributeSearch(a)) <> AttributeValue(a) then
                            warnlog "Field: " & AttributeSearch(a) & " is not " & AttributeValue(a) & "!"
                        end if         
                        SAXSeekElement("/")
                    case 35, 36, 37, 38 :
                        ' One node has four attributes          
                        sPathToAttributes = fWhereIsXMLElementInBody("office:document-content" , "office:body" , sXMLNodename(35))
                        SAXSeekElement(sPathToAttributes)                      
                        printlog " +- [Element: " & sXMLNodename(35) & "]" & AttributeSearch(a)           
                        if SAXGetAttributeValue(AttributeSearch(a)) <> AttributeValue(a) then
                            warnlog "Field: " & AttributeSearch(a) & " is not " & AttributeValue(a) & "!"
                        end if         
                        SAXSeekElement("/")
                    case 39, 40, 41, 42, 43 :
                        ' One node has five attributes          
                        sPathToAttributes = fWhereIsXMLElementInBody("office:document-content" , "office:body" , sXMLNodename(39))
                        SAXSeekElement(sPathToAttributes)                      
                        printlog " +- [Element: " & sXMLNodename(39) & "]" & AttributeSearch(a)           
                        if SAXGetAttributeValue(AttributeSearch(a)) <> AttributeValue(a) then
                            warnlog "Field: " & AttributeSearch(a) & " is not " & AttributeValue(a) & "!"
                        end if         
                        SAXSeekElement("/")
                    case 25, 26     : 
                        'skipping (evaluating different style names under different operating systems
                        SAXSeekElement("/")
                    case else   :
                        sPathToAttributes = fWhereIsXMLElementInBody("office:document-content" , "office:body" , sXMLNodename(a))
                        SAXSeekElement(sPathToAttributes)                      
                        printlog " +- [Element: " & sXMLNodename(a) & "]" & AttributeSearch(a)           
                        if SAXGetAttributeValue(AttributeSearch(a)) <> AttributeValue(a) then
                            warnlog "Field: " & AttributeSearch(a) & " is not " & AttributeValue(a) & "!"
                        end if         
                        SAXSeekElement("/")          
                end select
                printlog " |"                       
            next a      
            SAXRelease()
            printlog "    * * *"
        end if
    else
        call hCloseDocument
    end if
endcase
'
'-------------------------------------------------------------------------
'
testcase tfootnotes 
    Dim a as integer
    Dim i as integer
    Dim AttributeSearch(60) as string
    Dim AttributeValue(60) as string
    Dim sXMLNodename(50) as string
    Dim sPathToAttributes as string

    printlog "+- footnotes.sdw"
    '/// Opening file footnotes.sdw from input-directory.
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\writer\footnotes.sdw"))
    '/// Saving the document in OOo XML 1.0 format.
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\writer\level1\footnotes.sxw") , "StarOffice XML (Writer)") then    
        '/// Closing the document also if there is a verification dialog.    
        call hCloseDocument()    
        sleep(3)
        '/// Unpack the complete content of the file (ZIP-archieve) into a seperate directory.
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\writer\level1\footnotes.sxw") , gOfficePath & ConvertPath("user\work\xml\writer\level1\footnotes") )
        '/// Checking the well formness of content.xml, styles.xml and meta.xml.
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\writer\level1\footnotes\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\writer\level1\footnotes\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\writer\level1\footnotes\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            '/// Opening the exported file (GPF check).
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\writer\level1\footnotes.sxw"))
            sleep(2)
            '/// Closing the document.
            call hCloseDocument     
        end if
    else
        call hCloseDocument
    end if
endcase


