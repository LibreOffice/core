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
'* short description : XML Impress Include File
'*
'\***********************************************************************************
   Dim Isliste(250) as string
   Dim OutputPath as string

sub sxi7_01

   printlog "------------------- sxi_01.inc ---------------------"
   printlog "----------------------------------------------------"
   call talien_attributes
   printlog "---------------------- C J K -----------------------"
   call tchinese
   call tjapan
   call tkorean
   call tradchinese
   printlog "----------------------------------------------------"      
   call tAutolayout   
   call tText
   call tObjects
   call tEffects_text_objects
   call tEffects_slides   

end sub
'
'-------------------------------------------------------------------------
'
testcase talien_attributes
    Dim i as integer
    Dim AttrNameInDOM as string
    Dim AttributeSearch(9) as string
    Dim AttributeValue(9) as string
    printlog "+- Impress: alien.sxi"
    '/// Opening alien.sxi. ///
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\graphics\ooo10\alien.sxi"))
    '/// Saving document. ///
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\impress\level1\alien.sxi") , "StarOffice XML (Impress)") then
        '/// Closing document. ///
        call hCloseDocument
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\impress\level1\alien.sxi") , gOfficePath & ConvertPath("user\work\xml\impress\level1\alien\") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\impress\level1\alien\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\impress\level1\alien\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\impress\level1\alien\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            '/// Opening exported document (GPF check). ///
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\impress\level1\alien.sxi")
            sleep(2)
            '/// Closing document. ///
            call hCloseDocument
            sleep(2)
            SAXReadFile(gOfficePath & ConvertPath("user\work\xml\impress\level1\alien\content.xml"))
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
            
            AttributeSearch(3) = "alien:key"
            AttributeValue(3) = "shape"
            
            AttributeSearch(6) = "alien:key"
            AttributeValue(6) = "para"
            
            AttributeSearch(7) = "alien:key"
            AttributeValue(7) = "shape-text"
            
            AttributeSearch(9) = "alien:key"
            AttributeValue(9) = "text"
            '-------------------------------------------------------------------------
            printlog "++ Feature(s) to be searched for:"
            printlog " |"
            '-------------------------------------------------------------------------
            for i = 1 to 9
                if i = 1 then
                    printlog "-----------------------------------------"
                    printlog "Hidden attributes in a drawing-page style"
                    printlog "-----------------------------------------"
                end if
                if i = 3 then
                    printlog "-------------------------------------"
                    printlog "Hidden attributes in a graphics style"
                    printlog "-------------------------------------"
                end if
                if i = 6 or i = 7 then
                    printlog "-------------------------------------------"
                    printlog "Hidden attributes in a paragraph style (" & i-5 & ")"
                    printlog "-------------------------------------------"
                end if
                if i = 9 then
                    printlog "---------------------------------"
                    printlog "Hidden attributes in a text style"
                    printlog "---------------------------------"
                end if
                if i = 1 or i = 3 or i = 6 or i = 7 or i = 8 then
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
                        if i = 3 then 
                            warnlog "Hidden attributes (in a graphics style): " & AttributeSearch(i) & " is not " & AttributeValue(i) & "!"
                        end if
                        if i = 5 or i = 6 then 
                            warnlog "Hidden attributes (in a paragraph style): " & AttributeSearch(i) & " is not " & AttributeValue(i) & "!"
                        end if
                        if i = 8 then 
                            warnlog "Hidden attributes (in a text style): " & AttributeSearch(i) & " is not " & AttributeValue(i) & "!"
                        end if
                    end if
                end if
            next i
            SAXSeekElement("/")
            printlog "    * * *"
            '-------------------------------------------------------------------------
            SAXRelease()
            SAXReadFile(gOfficePath & ConvertPath("user\work\xml\impress\level1\alien\styles.xml"))
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
    printlog "+- Impress: chin.sxi"
    '/// Opening chin.sxi which includes chinese characters. ///
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\graphics\ooo10\chin.sxi"))
    '/// Saving document. ///
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\impress\level1\chin.sxi") , "StarOffice XML (Impress)") then
        '/// Closing document. ///
        call hCloseDocument
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\impress\level1\chin.sxi") , gOfficePath & ConvertPath("user\work\xml\impress\level1\chin\") )
        '/// Checking well formness of meta.xml, styles.xml and content.xml. ///
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\impress\level1\chin\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\impress\level1\chin\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\impress\level1\chin\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            '/// Opening the exported document again (GPF check). ///
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\impress\level1\chin.sxi")
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
    printlog "+- Impress: japan.sxi"
    '/// Opening japan.sxi which includes japanese characters. ///
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\graphics\ooo10\japan.sxi"))
    '/// Saving document. ///
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\impress\level1\japan.sxi") , "StarOffice XML (Impress)") then
        '/// Closing document. ///
        call hCloseDocument
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\impress\level1\japan.sxi") , gOfficePath & ConvertPath("user\work\xml\impress\level1\japan\") )
        '/// Checking well formness of meta.xml, styles.xml and content.xml. ///
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\impress\level1\japan\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\impress\level1\japan\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\impress\level1\japan\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            '/// Opening the exported document again (GPF check). ///
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\impress\level1\japan.sxi")
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
testcase tkorean
    printlog "+- Impress: korean.sxi"
    '/// Opening korean.sxi which includes korean characters. ///
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\graphics\ooo10\korean.sxi"))
    '/// Saving document. ///
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\impress\level1\korean.sxi") , "StarOffice XML (Impress)") then
        '/// Closing document. ///
        call hCloseDocument
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\impress\level1\korean.sxi") , gOfficePath & ConvertPath("user\work\xml\impress\level1\korean\") )
        '/// Checking well formness of meta.xml, styles.xml and content.xml. ///
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\impress\level1\korean\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\impress\level1\korean\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\impress\level1\korean\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            '/// Opening the exported document again (GPF check). ///
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\impress\level1\korean.sxi")
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
    printlog "+- Impress: trad_chin.sxi"
    '/// Opening trad_chin.sxi which includes traditional chinese characters. ///
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\graphics\ooo10\trad_chin.sxi"))
    '/// Saving document. ///
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\impress\level1\trad_chin.sxi") , "StarOffice XML (Impress)") then
        '/// Closing document. ///
        call hCloseDocument
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\impress\level1\trad_chin.sxi") , gOfficePath & ConvertPath("user\work\xml\impress\level1\trad_chin\") )
        '/// Checking well formness of meta.xml, styles.xml and content.xml. ///
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\impress\level1\trad_chin\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\impress\level1\trad_chin\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\impress\level1\trad_chin\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            '/// Opening the exported document again (GPF check). ///
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\impress\level1\trad_chin.sxi")
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
testcase tAutolayout
    Dim AttributeSearch(8) as string
    Dim AttributeValue(8) as string
    printlog "+- Impress: Autolayout.sdd"
    '/// Opening Autolayout.sdd. ///
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\graphics\so_bin\Autolayout.sdd"))
    '/// Saving document. ///
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\impress\level1\Autolayout.sxi") , "StarOffice XML (Impress)") then
        '/// Closing document. ///
        call hCloseDocument
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\impress\level1\Autolayout.sxi") , gOfficePath & ConvertPath("user\work\xml\impress\level1\Autolayout\") )
        '/// Checking well formness of meta.xml, styles.xml and content.xml. ///
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\impress\level1\Autolayout\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\impress\level1\Autolayout\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\impress\level1\Autolayout\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            '/// Opening the exported document again (GPF check). ///
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\impress\level1\Autolayout.sxi")
            sleep(2)
            '/// Closing document. ///
            call hCloseDocument
            sleep(2)
            SAXReadFile(gOfficePath & ConvertPath("user\work\xml\impress\level1\Autolayout\content.xml"))
            printlog "-----------------"
            printlog "Slide names"
            printlog "-----------------"
            '/// <u><b>content.xml</u></b>
            '/// <u>Slide names</u>
            '///+ Searching for: draw:name="Title subtitle"
            '-------------------------------------------------------------------------
            AttributeSearch(1) = "draw:name"
            AttributeValue(1) = "Title subtitle"
            printlog "++ Feature(s) to be searched for:"
            printlog " |"
            '-------------------------------------------------------------------------
            SAXSeekElement("office:document-content")
            SAXSeekElement("office:body")
            SAXSeekElement(2)
            printlog " +- " & AttributeSearch(1) & " = " & AttributeValue(1)
            if SAXGetAttributeValue(AttributeSearch(1)) <> AttributeValue(1) then
                warnlog "draw:name: " & AttributeSearch(1) & " is not " & AttributeValue(1) & "!"
            end if            
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
testcase tText
    printlog "+- Impress: Text.sdd"
    QAErrorLog "#i38739# tText PERFORMANCE issue."
    goto endsub
    '/// Opening Text.sdd. ///
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\graphics\so_bin\Text.sdd"))
    '/// Saving document. ///
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\impress\level1\Text.sxi") , "StarOffice XML (Impress)") then
        '/// Closing document. ///
        call hCloseDocument
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\impress\level1\Text.sxi") , gOfficePath & ConvertPath("user\work\xml\impress\level1\Text\") )
        '/// Checking well formness of meta.xml, styles.xml and content.xml. ///
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\impress\level1\Text\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\impress\level1\Text\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\impress\level1\Text\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            '/// Opening the exported document again (GPF check). ///
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\impress\level1\Text.sxi")
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
testcase tObjects
    printlog "+- Impress: Objects.sdd"
    '/// Opening Objects.sdd. ///
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\graphics\so_bin\Objects.sdd"))
    '/// Saving document. ///
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\impress\level1\Objects.sxi") , "StarOffice XML (Impress)") then
        '/// Closing document. ///
        call hCloseDocument
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\impress\level1\Objects.sxi") , gOfficePath & ConvertPath("user\work\xml\impress\level1\Objects\") )
        '/// Checking well formness of meta.xml, styles.xml and content.xml. ///
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\impress\level1\Objects\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\impress\level1\Objects\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\impress\level1\Objects\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            '/// Opening the exported document again (GPF check). ///
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\impress\level1\Objects.sxi")
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
testcase tEffects_text_objects
    printlog "+- Impress: Effects_text_objects.sdd"
    '/// Opening Effects_text_objects.sdd. ///
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\graphics\so_bin\Effects_text_objects.sdd"))
    '/// Saving document. ///
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\impress\level1\Effects_text_objects.sxi") , "StarOffice XML (Impress)") then
        '/// Closing document. ///
        call hCloseDocument
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\impress\level1\Effects_text_objects.sxi") , gOfficePath & ConvertPath("user\work\xml\impress\level1\Effects_text_objects\") )
        '/// Checking well formness of meta.xml, styles.xml and content.xml. ///
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\impress\level1\Effects_text_objects\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\impress\level1\Effects_text_objects\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\impress\level1\Effects_text_objects\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            '/// Opening the exported document again (GPF check). ///
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\impress\level1\Effects_text_objects.sxi")
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
testcase tEffects_slides
    printlog "+- Impress: Effects_slides.sdd"
    '/// Opening Effects_slides.sdd. ///
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\graphics\so_bin\Effects_slides.sdd"))
    '/// Saving document. ///
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\impress\level1\Effects_slides.sxi") , "StarOffice XML (Impress)") then
        '/// Closing document. ///
        call hCloseDocument
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\impress\level1\Effects_slides.sxi") , gOfficePath & ConvertPath("user\work\xml\impress\level1\Effects_slides\") )
        '/// Checking well formness of meta.xml, styles.xml and content.xml. ///
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\impress\level1\Effects_slides\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\impress\level1\Effects_slides\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\impress\level1\Effects_slides\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            '/// Opening the exported document again (GPF check). ///
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\impress\level1\Effects_slides.sxi")
            sleep(2)
            '/// Closing document. ///
            call hCloseDocument
            sleep(2)
        end if
    else
        call hCloseDocument
    end if
endcase

