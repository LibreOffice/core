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
'* short description : XML Draw Include File
'*
'\***********************************************************************************
   Dim Isliste(250) as string
   Dim OutputPath as string

sub sxd7_01
   printlog "------------------- sxd_01.inc ---------------------"
   printlog "----------------------------------------------------"
   call talien_attributes
   printlog "---------------------- C J K -----------------------"
   call tchinese
   call tjapan
   call tkorean
   call tradchinese
   printlog "----------------------------------------------------"
   call t18092
   call t18093
   call t18094
   call t18098
   call t18099
   call t18100
   call t18101
   call t18102
   call t18106
   call t18107

end sub

'-------------------------------------------------------------------------

testcase talien_attributes
    Dim i as integer
    Dim AttrNameInDOM as string
    Dim AttributeSearch(5) as string
    Dim AttributeValue(5) as string
    printlog "+- Impress: alien.sxd"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\graphics\ooo10\alien.sxd"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\draw\level1\alien.sxd") , "StarOffice XML (Draw)") then
        call hCloseDocument
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\draw\level1\alien.sxd") , gOfficePath & ConvertPath("user\work\xml\draw\level1\alien\") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\alien\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\alien\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\alien\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\draw\level1\alien.sxd")
            sleep(2)
            call hCloseDocument
            sleep(2)
            SAXReadFile(gOfficePath & ConvertPath("user\work\xml\draw\level1\alien\content.xml"))
            printlog "-----------------"
            printlog "Hidden Namespaces"
            printlog "-----------------"
            '/// <u><b>content.xml</u></b>
            '/// <u>Hidden attributes in namespaces</u>
            '///+ Searching for: xmlns:alien=&quot;http://openoffice.org/2000/alien&quot;
            '-------------------------------------------------------------------------
            AttributeSearch(1) = "xmlns:alien"
            AttributeValue(1) = "http://openoffice.org/2000/alien"
            printlog "++ Feature(s) to be searched for:"
            printlog " |"
            '-------------------------------------------------------------------------
            SAXSeekElement("office:document-content")
            printlog " +- " & AttributeSearch(1)
            if SAXGetAttributeValue(AttributeSearch(1)) <> AttributeValue(1) then
                warnlog "Hidden attributes (in namespace): " & AttributeSearch(1) & " is not " & AttributeValue(1) & "!"
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
            AttributeSearch(1) = "alien:key"
            AttributeValue(1) = "page"
            
            AttributeSearch(2) = "alien:key"
            AttributeValue(2) = "shape"
            
            AttributeSearch(3) = "alien:key"
            AttributeValue(3) = "para"
            
            AttributeSearch(4) = "alien:key"
            AttributeValue(4) = "shape-text"
            
            AttributeSearch(5) = "alien:key"
            AttributeValue(5) = "text"
            '-------------------------------------------------------------------------
            printlog "++ Feature(s) to be searched for:"
            printlog " |"
            '-------------------------------------------------------------------------
            for i = 1 to 5
                if i = 1 then
                    printlog "-----------------------------------------"
                    printlog "Hidden attributes in a drawing-page style"
                    printlog "-----------------------------------------"
                end if
                if i = 2 then
                    printlog "-------------------------------------"
                    printlog "Hidden attributes in a graphics style"
                    printlog "-------------------------------------"
                end if
                if i = 3 or i = 4 then
                    printlog "-------------------------------------------"
                    printlog "Hidden attributes in a paragraph style (" & i-2 & ")"
                    printlog "-------------------------------------------"
                end if
                if i = 5 then
                    printlog "---------------------------------"
                    printlog "Hidden attributes in a text style"
                    printlog "---------------------------------"
                end if
                'The hidden attributes are in a known sequence included,
                'so the same routine will be used for different styles!
                SAXSeekElement("/")
                SAXSeekElement("office:document-content")
                SAXSeekElement("office:automatic-styles")
                SAXSeekElement("style:style" , i )
                AttrNameInDOM = SAXGetAttributeValue("style:name")
                printlog " + Style name (" & i & "): " & AttrNameInDOM
                printlog " |"
                SAXSeekElement("style:properties")
                printlog " +- " & AttributeSearch(i)
                if SAXGetAttributeValue(AttributeSearch(i)) <> AttributeValue(i) then
                    'i = <nr> means <nr>'th style:style in DOM tree
                    if i = 1 then 
                        warnlog "Hidden attributes (in a drawing-page style): " & AttributeSearch(i) & " is not " & AttributeValue(i) & "!"
                    end if
                    if i = 2 then 
                        warnlog "Hidden attributes (in a graphics style): " & AttributeSearch(i) & " is not " & AttributeValue(i) & "!"
                    end if
                    if i = 3 or i = 4 then 
                        warnlog "Hidden attributes (in a paragraph style): " & AttributeSearch(i) & " is not " & AttributeValue(i) & "!"
                    end if
                    if i = 5 then 
                        warnlog "Hidden attributes (in a text style): " & AttributeSearch(i) & " is not " & AttributeValue(i) & "!"
                    end if
                end if
            next i
            SAXSeekElement("/")
            printlog "    * * *"
            '-------------------------------------------------------------------------
            SAXRelease()
            SAXReadFile(gOfficePath & ConvertPath("user\work\xml\draw\level1\alien\styles.xml"))
            '-------------------------------------------------------------------------
            printlog "    -------------------"
            printlog "    s t y l e s . x m l"
            printlog "    -------------------"
            printlog "-----------------"
            printlog "Hidden Namespaces"
            printlog "-----------------"
            '/// <u><b>styles.xml</u></b>
            '/// <u>Hidden attributes in namespaces</u>
            '///+ Searching for: xmlns:alien=&quot;http://openoffice.org/2000/alien&quot;
            '-------------------------------------------------------------------------
            AttributeSearch(1) = "xmlns:alien"
            AttributeValue(1) = "http://openoffice.org/2000/alien"
            printlog "++ Feature(s) to be searched for:"
            printlog " |"
            '-------------------------------------------------------------------------
            SAXSeekElement("office:document-styles")
            printlog " +- " & AttributeSearch(1)
            if SAXGetAttributeValue(AttributeSearch(1)) <> AttributeValue(1) then
                warnlog "Hidden attributes (in namespace): " & AttributeSearch(1) & " is not " & AttributeValue(1) & "!"
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
            AttributeSearch(1) = "alien:key"
            AttributeValue(1) = "master-page"
            printlog "++ Feature(s) to be searched for:"
            printlog " |"
            '-------------------------------------------------------------------------
            SAXSeekElement("office:document-styles")
            SAXSeekElement("office:automatic-styles")
            SAXSeekElement("style:style")
            SAXSeekElement("style:properties")
            printlog " +- " & AttributeSearch(1)
            if SAXGetAttributeValue(AttributeSearch(1)) <> AttributeValue(1) then
                warnlog "Hidden attributes (in master-page style): " & AttributeSearch(1) & " is not " & AttributeValue(1) & "!"
            end if
            printlog "    * * *"
            '-------------------------------------------------------------------------
            SAXRelease()
        end if
    else
        call hCloseDocument    
    end if
endcase
'
'-------------------------------------------------------------------------
'
testcase tchinese
    printlog "+- Impress: chin.sxd"
    '/// Opening chin.sxd which includes chinese characters. ///
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\graphics\ooo10\chin.sxd"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\draw\level1\chin.sxd") , "StarOffice XML (Draw)") then
        '/// Saving it. ///
        call hCloseDocument
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\draw\level1\chin.sxd") , gOfficePath & ConvertPath("user\work\xml\draw\level1\chin\") )
        '/// Checking well formness of meta.xml, styles.xml and content.xml. ///
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\chin\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\chin\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\chin\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            '/// Opening the exported document again (GPF check). ///
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\draw\level1\chin.sxd")
            sleep(2)
            '/// Closing document. ///
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
testcase tjapan
    printlog "+- Impress: japan.sxd"
    '/// Opening japan.sxd which includes japanese characters. ///
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\graphics\ooo10\japan.sxd"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\draw\level1\japan.sxd") , "StarOffice XML (Draw)") then
        '/// Saving it. ///
        call hCloseDocument
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\draw\level1\japan.sxd") , gOfficePath & ConvertPath("user\work\xml\draw\level1\japan\") )
        '/// Checking well formness of meta.xml, styles.xml and content.xml. ///
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\japan\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\japan\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\japan\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            '/// Opening the exported document again (GPF check). ///
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\draw\level1\japan.sxd")
            sleep(2)
            call hCloseDocument
            '/// Closing document. ///
            sleep(2)
        end if
    else
        call hCloseDocument
    end if
endcase
'
'-------------------------------------------------------------------------
'
testcase tkorean
    printlog "+- Impress: korean.sxd"
    '/// Opening korean.sxd which includes korean characters. ///
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\graphics\ooo10\korean.sxd"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\draw\level1\korean.sxd") , "StarOffice XML (Draw)") then
        '/// Saving it. ///
        call hCloseDocument
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\draw\level1\korean.sxd") , gOfficePath & ConvertPath("user\work\xml\draw\level1\korean\") )
        '/// Checking well formness of meta.xml, styles.xml and content.xml. ///
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\korean\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\korean\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\korean\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            '/// Opening the exported document again (GPF check). ///
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\draw\level1\korean.sxd")
            sleep(2)
            '/// Closing document. ///
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
testcase tradchinese
    printlog "+- Impress: trad_chin.sxd"
    '/// Opening trad_chin.sxd which includes traditional chinese characters. ///
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\graphics\ooo10\trad_chin.sxd"))
    '/// Saving it. ///
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\draw\level1\trad_chin.sxd") , "StarOffice XML (Draw)") then
        call hCloseDocument
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\draw\level1\trad_chin.sxd") , gOfficePath & ConvertPath("user\work\xml\draw\level1\trad_chin\") )
        '/// Checking well formness of meta.xml, styles.xml and content.xml. ///
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\trad_chin\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\trad_chin\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\trad_chin\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
        '/// Opening the exported document again (GPF check). ///
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\draw\level1\trad_chin.sxd")
            sleep(2)
            '/// Closing document. ///
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
testcase t18092
    printlog "+- Draw: 18092.sda"

    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\graphics\so_bin\18092.sda"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\draw\level1\18092.sxd") , "StarOffice XML (Draw)") then
        call hCloseDocument
        sleep(2)
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\draw\level1\18092.sxd") , gOfficePath & ConvertPath("user\work\xml\draw\level1\18092\") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18092\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18092\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18092\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\draw\level1\18092.sxd")
            call hCloseDocument
        end if
    else    
        call hCloseDocument
    end if
endcase
'
'-------------------------------------------------------------------------
'
testcase t18093
    printlog "+- Draw: 18093.sda"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\graphics\so_bin\18093.sda"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\draw\level1\18093.sxd") , "StarOffice XML (Draw)") then
        call hCloseDocument
        sleep(2)
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\draw\level1\18093.sxd") , gOfficePath & ConvertPath("user\work\xml\draw\level1\18093\") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18093\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18093\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18093\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\draw\level1\18093.sxd")
            call hCloseDocument
        end if
    else
        call hCloseDocument
    end if
endcase
'
'-------------------------------------------------------------------------
'
testcase t18094
    printlog "+- Draw: 18094.sda"

    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\graphics\so_bin\18094.sda"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\draw\level1\18094.sxd") , "StarOffice XML (Draw)") then
        call hCloseDocument
        sleep(2)
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\draw\level1\18094.sxd") , gOfficePath & ConvertPath("user\work\xml\draw\level1\18094\") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18094\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18094\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18094\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\draw\level1\18094.sxd")
            call hCloseDocument
        end if
    else
        call hCloseDocument
    end if
endcase
'
'-------------------------------------------------------------------------
'
testcase t18098
    printlog "+- Draw: 18098.sda"

    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\graphics\so_bin\18098.sda"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\draw\level1\18098.sxd") , "StarOffice XML (Draw)") then
        call hCloseDocument
        sleep(2)
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\draw\level1\18098.sxd") , gOfficePath & ConvertPath("user\work\xml\draw\level1\18098\") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18098\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18098\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18098\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\draw\level1\18098.sxd")
            call hCloseDocument
        end if
    else
        call hCloseDocument
    end if
endcase
'
'-------------------------------------------------------------------------
'
testcase t18099
    printlog "+- Draw: 18099.sda"

    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\graphics\so_bin\18099.sda"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\draw\level1\18099.sxd") , "StarOffice XML (Draw)") then
        call hCloseDocument
        sleep(2)
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\draw\level1\18099.sxd") , gOfficePath & ConvertPath("user\work\xml\draw\level1\18099\") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18099\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18099\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18099\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\draw\level1\18099.sxd")
            call hCloseDocument
        end if
    else
        call hCloseDocument
    end if
endcase
'
'-------------------------------------------------------------------------
'
testcase t18100
    printlog "+- Draw: 18100.sda"

    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\graphics\so_bin\18100.sda"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\draw\level1\18100.sxd") , "StarOffice XML (Draw)") then
        call hCloseDocument
        sleep(2)
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\draw\level1\18100.sxd") , gOfficePath & ConvertPath("user\work\xml\draw\level1\18100\") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18100\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18100\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18100\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\draw\level1\18100.sxd")
            call hCloseDocument
        end if
    else
        call hCloseDocument
    end if
endcase
'
'-------------------------------------------------------------------------
'
testcase t18101
    printlog "+- Draw: 18101.sda"

    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\graphics\so_bin\18101.sda"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\draw\level1\18101.sxd") , "StarOffice XML (Draw)") then
        call hCloseDocument
        sleep(2)
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\draw\level1\18101.sxd") , gOfficePath & ConvertPath("user\work\xml\draw\level1\18101\") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18101\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18101\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18101\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\draw\level1\18101.sxd")
            call hCloseDocument
        end if
    else
        call hCloseDocument    
    end if
endcase
'
'-------------------------------------------------------------------------
'
testcase t18102
    printlog "+- Draw: 18102.sda"

    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\graphics\so_bin\18102.sda"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\draw\level1\18102.sxd") , "StarOffice XML (Draw)") then
        call hCloseDocument
        sleep(2)
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\draw\level1\18102.sxd") , gOfficePath & ConvertPath("user\work\xml\draw\level1\18102\") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18102\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18102\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18102\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\draw\level1\18102.sxd")
            call hCloseDocument
        end if
    else
        call hCloseDocument
    end if
endcase
'
'-------------------------------------------------------------------------
'
testcase t18106
    printlog "+- Draw: 18106.sda"

    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\graphics\so_bin\18106.sda"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\draw\level1\18106.sxd") , "StarOffice XML (Draw)") then
        call hCloseDocument
        sleep(2)
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\draw\level1\18106.sxd") , gOfficePath & ConvertPath("user\work\xml\draw\level1\18106\") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18106\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18106\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18106\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\draw\level1\18106.sxd")
            call hCloseDocument
        end if
    else
        call hCloseDocument
    end if
endcase
'
'-------------------------------------------------------------------------
'
testcase t18107
    printlog "+- Draw: 18107.sda"

    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\graphics\so_bin\18107.sda"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\draw\level1\18107.sxd") , "StarOffice XML (Draw)") then
        call hCloseDocument
        sleep(2)
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\draw\level1\18107.sxd") , gOfficePath & ConvertPath("user\work\xml\draw\level1\18107\") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18107\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18107\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18107\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\draw\level1\18107.sxd")
            call hCloseDocument
        end if
    else
        call hCloseDocument
    end if
endcase

