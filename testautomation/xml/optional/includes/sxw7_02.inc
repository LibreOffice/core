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

sub sxw7_02

   printlog "-------------------------- SXW7_02.INC ------------------------------"   
   call tframes
   call hEnablePrettyPrinting(1)
   printlog "---------------------------   C J K    ------------------------------"
   call tschina
   call ttchina
   call tkorean
   call tjapan

end sub
'
'-------------------------------------------------------------------------
'
testcase tframes
    'NOTE: This test has been created as there were no SAX parser available for using
    '      in testtools. It is very (!) time consuming to debug this routines.
    '      Feel free to contact me if there is any issue with this script! jsi@openoffice.org
    Dim Searchstring(20) as string
    Dim StyleBodyA as string
    Dim StyleDefinitionA as string
    Dim i as integer
    Dim a as integer
    Dim AdditionalParameter as string
    '/// Disabling the 'pretty printing' functionality.
    call hEnablePrettyPrinting()
    printlog "+- frames.sdw"
    '/// Opening frames.sdw from input directory.
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\writer\frames.sdw"))
    '/// Saving document as OOo XML 1.0 file.
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\writer\level1\frames.sxw") , "StarOffice XML (Writer)") then
        '/// Closing the document also if there is a verification dialog.
        call hCloseDocument()
        sleep(3)
        '/// Unpacking the file (ZIP archieve) into a seperate directory. 
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\writer\level1\frames.sxw") , gOfficePath & ConvertPath("user\work\xml\writer\level1\frames") )
        '/// Verifying well-formness of content.xml, styles.xml and meta.xml.
        if XMLWellFormed (gOfficePath & ConvertPath("user\work\xml\writer\level1\frames\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellFormed (gOfficePath & ConvertPath("user\work\xml\writer\level1\frames\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellFormed (gOfficePath & ConvertPath("user\work\xml\writer\level1\frames\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\writer\level1\frames.sxw"))
            sleep(2)
            call hCloseDocument
            sleep(2)
            printlog "-------------------------------------------------------------------------"
            Searchstring(1) = "<draw:text-box draw:style-name=" & CHR$(34)
            Searchstring(2) = "draw:name=" & CHR$(34)
            Searchstring(3) = "text:anchor-type=" & CHR$(34) & "page" & CHR$(34)
            Searchstring(4) = "text:anchor-page-number=" & CHR$(34) & "1" & CHR$(34)
            Searchstring(5) = "svg:x=" & CHR$(34)
            Searchstring(6) = "svg:y=" & CHR$(34)
            Searchstring(7) = "svg:width=" & CHR$(34)
            Searchstring(8) = "fo:min-height=" & CHR$(34)
            Searchstring(9) = "draw:z-index=" & CHR$(34)
            printlog "++ Feature(s) to be searched for:"
            printlog " |"
            '-------------------------------------------------------------------------
            printlog " ..searching in 'draw:text-box' number 1"     
            AdditionalParameter = GetLineInXMLBody(gOfficePath & ConvertPath("user\work\xml\writer\level1\frames\content.xml") , "draw:text-box" , 1)
            For i = 1 to 9
                printlog " +- " & Searchstring(i)
                if InStr (AdditionalParameter , Searchstring(i)) = 0 then
                    warnlog "Draw Text Box (Frame): " & Searchstring(i) & " not found!"
                end if
            Next i
            printlog "-------------------------------------------------------------------------"            
            Searchstring(1) = ">Anchor to page<"
            printlog " +- " & Searchstring(1) 
            AdditionalParameter = GetLineInXMLBody(gOfficePath & ConvertPath("user\work\xml\writer\level1\frames\content.xml") , "text:p" , 1)     
            printlog " +- " & Searchstring(1)
            if InStr (AdditionalParameter , Searchstring(1)) = 0 then
                warnlog "Draw Text Box (Frame) Content: " & Searchstring(1) & " not found!"
            end if
            printlog "-------------------------------------------------------------------------"         
            Searchstring(1) = "<draw:text-box"
            Searchstring(2) = "draw:style-name=" & CHR$(34)
            Searchstring(3) = "draw:name=" & CHR$(34)
            Searchstring(4) = "text:anchor-type=" & CHR$(34) & "page" & CHR$(34)
            Searchstring(5) = "text:anchor-page-number=" & CHR$(34) & "1" & CHR$(34)
            Searchstring(6) = "svg:x=" & CHR$(34)
            Searchstring(7) = "svg:y=" & CHR$(34)
            Searchstring(8) = "svg:width=" & CHR$(34)
            Searchstring(9) = "fo:min-height=" & CHR$(34)
            Searchstring(10) = "draw:z-index=" & CHR$(34)
            For a = 1 to 5
                printlog " ..searching in 'draw:text-box' number " & a
                AdditionalParameter = GetLineInXMLBody(gOfficePath & ConvertPath("user\work\xml\writer\level1\frames\content.xml") , "draw:text-box" , a)
                For i = 1 to 10
                    printlog " +- " & Searchstring(i)
                    if InStr (AdditionalParameter , Searchstring(i)) = 0 then
                        warnlog "Draw Text Box (Frame): " & Searchstring(i) & " not found!"
                    end if
                Next i
            Next a
            printlog "-------------------------------------------------------------------------"
            Searchstring(1) = ">No wrap<"
            printlog " +- " & Searchstring(1)            
            AdditionalParameter = GetLineInXMLBody(gOfficePath & ConvertPath("user\work\xml\writer\level1\frames\content.xml") , "text:p" , 2)
            printlog " +- " & Searchstring(1)
            if InStr (AdditionalParameter , Searchstring(1)) = 0 then
                warnlog "Draw Text Box (Frame) Content: " & Searchstring(1) & " not found!"
            end if
            printlog "-------------------------------------------------------------------------"
            Searchstring(1) = ">Page wrap<"
            printlog " +- " & Searchstring(1)
            '-------------------------------------------------------------------------
            AdditionalParameter = GetLineInXMLBody(gOfficePath & ConvertPath("user\work\xml\writer\level1\frames\content.xml") , "text:p" , 3)
            printlog " +- " & Searchstring(1)
            if InStr (AdditionalParameter , Searchstring(1)) = 0 then
                warnlog "Draw Text Box (Frame) Content: " & Searchstring(1) & " not found!"
            end if
            printlog "-------------------------------------------------------------------------"
            Searchstring(1) = ">Optimal<"
            printlog " +- " & Searchstring(1)            
            AdditionalParameter = GetLineInXMLBody(gOfficePath & ConvertPath("user\work\xml\writer\level1\frames\content.xml") , "text:p" , 4)
            printlog " +- " & Searchstring(1)
            if InStr (AdditionalParameter , Searchstring(1)) = 0 then
                warnlog "Draw Text Box (Frame) Content: " & Searchstring(1) & " not found!"
            end if
            printlog "-------------------------------------------------------------------------"
            Searchstring(1) = ">Wrap<"
            printlog " +- " & Searchstring(1)            
            AdditionalParameter = GetLineInXMLBody(gOfficePath & ConvertPath("user\work\xml\writer\level1\frames\content.xml") , "text:p" , 5)
            printlog " +- " & Searchstring(1)
            if InStr (AdditionalParameter , Searchstring(1)) = 0 then
                warnlog "Draw Text Box (Frame) Content: " & Searchstring(1) & " not found!"
            end if
            printlog "-------------------------------------------------------------------------"
            Searchstring(1) = ">Background<"
            printlog " +- " & Searchstring(1)            
            AdditionalParameter = GetLineInXMLBody(gOfficePath & ConvertPath("user\work\xml\writer\level1\frames\content.xml") , "text:p" , 6)
            printlog " +- " & Searchstring(1)
            if InStr (AdditionalParameter , Searchstring(1)) = 0 then
                warnlog "Draw Text Box (Frame) Content: " & Searchstring(1) & " not found!"
            end if
            printlog "-------------------------------------------------------------------------"
            Searchstring(1) = "<draw:text-box"
            Searchstring(2) = "draw:style-name=" & CHR$(34)
            Searchstring(3) = "draw:name=" & CHR$(34)
            Searchstring(4) = "text:anchor-type=" & CHR$(34) & "paragraph" & CHR$(34)
            Searchstring(5) = "svg:x=" & CHR$(34)
            Searchstring(6) = "svg:y=" & CHR$(34)
            Searchstring(7) = "svg:width=" & CHR$(34)
            Searchstring(8) = "fo:min-height=" & CHR$(34)
            Searchstring(9) = "draw:z-index=" & CHR$(34)
            AdditionalParameter = GetLineInXMLBody(gOfficePath & ConvertPath("user\work\xml\writer\level1\frames\content.xml") , "draw:text-box" , 7)
            For i = 1 to 9
                printlog " +- " & Searchstring(i)
                if InStr (AdditionalParameter , Searchstring(i)) = 0 then
                    warnlog "Draw Text Box (Frame): " & Searchstring(i) & " not found!"
                end if
            Next i
            printlog "-------------------------------------------------------------------------"
            Searchstring(1) = "draw:style-name=" & CHR$(34)
            Searchstring(2) = "draw:name=" & CHR$(34)
            Searchstring(3) = "text:anchor-type=" & CHR$(34) & "char" & CHR$(34)
            Searchstring(4) = "svg:y=" & CHR$(34)
            Searchstring(5) = "svg:width=" & CHR$(34)
            Searchstring(6) = "fo:min-height=" & CHR$(34)
            For a = 1 to 2
                printlog " ..searching in 'draw:text-box' number " & a+7
                AdditionalParameter = GetLineInXMLBody(gOfficePath & ConvertPath("user\work\xml\writer\level1\frames\content.xml") , "draw:text-box" , a+7)
                if a = 2 then
                    Searchstring(3) = "text:anchor-type=" & CHR$(34) & "as-char" & CHR$(34)
                end if
                For i = 1 to 6
                    printlog " +- " & Searchstring(i)
                    if InStr (AdditionalParameter , Searchstring(i)) = 0 then
                        warnlog "Draw Text Box (Frame): " & Searchstring(i) & " not found!"
                    end if
                Next i
            Next a
            printlog "-------------------------------------------------------------------------"
            Searchstring(1) = "<draw:text-box"
            Searchstring(2) = "draw:style-name=" & CHR$(34)
            Searchstring(3) = "draw:name=" & CHR$(34)
            Searchstring(4) = "text:anchor-type=" & CHR$(34) & "paragraph" & CHR$(34)
            Searchstring(5) = "svg:x=" & CHR$(34)
            Searchstring(6) = "svg:y=" & CHR$(34)
            Searchstring(7) = "svg:width=" & CHR$(34)
            Searchstring(8) = "fo:min-height=" & CHR$(34)
            AdditionalParameter = GetLineInXMLBody(gOfficePath & ConvertPath("user\work\xml\writer\level1\frames\content.xml") , "draw:text-box" , 10)
            For i = 1 to 8
                printlog " +- " & Searchstring(i)
                if InStr (AdditionalParameter , Searchstring(i)) = 0 then
                    warnlog "Draw Text Box (Frame): " & Searchstring(i) & " not found!"
                end if
            Next i
            printlog "-------------------------------------------------------------------------"
            Searchstring(1) = "<draw:text-box"
            Searchstring(2) = "draw:style-name=" & CHR$(34)
            Searchstring(3) = "draw:name=" & CHR$(34)
            Searchstring(4) = "text:anchor-type=" & CHR$(34) & "paragraph" & CHR$(34)
            Searchstring(5) = "svg:x=" & CHR$(34)
            Searchstring(6) = "svg:y=" & CHR$(34)
            Searchstring(7) = "svg:width=" & CHR$(34)
            Searchstring(8) = "fo:min-height=" & CHR$(34)
            Searchstring(9) = "draw:z-index=" & CHR$(34)
            AdditionalParameter = GetLineInXMLBody(gOfficePath & ConvertPath("user\work\xml\writer\level1\frames\content.xml") , "draw:text-box" , 11)
            For i = 1 to 9
                printlog " +- " & Searchstring(i)
                if InStr (AdditionalParameter , Searchstring(i)) = 0 then
                    warnlog "Draw Text Box (Frame): " & Searchstring(i) & " not found!"
                end if
            Next i
            printlog "-------------------------------------------------------------------------"
            Searchstring(1) = "<draw:text-box"
            Searchstring(2) = "draw:style-name=" & CHR$(34)
            Searchstring(3) = "draw:name=" & CHR$(34)
            Searchstring(4) = "text:anchor-type=" & CHR$(34) & "paragraph" & CHR$(34)
            Searchstring(5) = "svg:x=" & CHR$(34)
            Searchstring(6) = "svg:y=" & CHR$(34)
            Searchstring(7) = "svg:width=" & CHR$(34)
            Searchstring(8) = "fo:min-height=" & CHR$(34)
            Searchstring(9) = "draw:z-index=" & CHR$(34)
            AdditionalParameter = GetLineInXMLBody(gOfficePath & ConvertPath("user\work\xml\writer\level1\frames\content.xml") , "draw:text-box" , 12)
            For i = 1 to 9
                printlog " +- " & Searchstring(i)
                if InStr (AdditionalParameter , Searchstring(i)) = 0 then
                    warnlog "Draw Text Box (Frame): " & Searchstring(i) & " not found!"
                end if
            Next i
            printlog "-------------------------------------------------------------------------"
            Searchstring(1) = "<draw:text-box"
            Searchstring(2) = "draw:style-name=" & CHR$(34)
            Searchstring(3) = "draw:name=" & CHR$(34)
            Searchstring(4) = "text:anchor-type=" & CHR$(34) & "paragraph" & CHR$(34)
            Searchstring(5) = "svg:x=" & CHR$(34)
            Searchstring(6) = "svg:y=" & CHR$(34)
            Searchstring(7) = "svg:width=" & CHR$(34)
            Searchstring(8) = "fo:min-height=" & CHR$(34)
            Searchstring(9) = "draw:z-index=" & CHR$(34)
            AdditionalParameter = GetLineInXMLBody(gOfficePath & ConvertPath("user\work\xml\writer\level1\frames\content.xml") , "draw:text-box" , 13)
            For i = 1 to 9
                printlog " +- " & Searchstring(i)
                if InStr (AdditionalParameter , Searchstring(i)) = 0 then
                    warnlog "Draw Text Box (Frame): " & Searchstring(i) & " not found!"
                end if
            Next i
            printlog "-------------------------------------------------------------------------"               
            '    -----------------------
            '       Frame S T Y L E S
            '    -----------------------   
            '-------------------------------------------------------------------------
            Searchstring(1) = "style:family=" & CHR$(34) & "graphics" & CHR$(34)
            Searchstring(2) = "style:parent-style-name=" & CHR$(34)
            Searchstring(3) = "style:wrap=" & CHR$(34) & "parallel" & CHR$(34)
            Searchstring(4) = "style:number-wrapped-paragraphs=" & CHR$(34) & "no-limit" & CHR$(34)
            Searchstring(5) = "style:vertical-pos=" & CHR$(34) & "from-top" & CHR$(34)
            Searchstring(6) = "style:vertical-rel=" & CHR$(34) & "page" & CHR$(34)
            Searchstring(7) = "style:horizontal-pos=" & CHR$(34) & "from-left" & CHR$(34)
            Searchstring(8) = "style:horizontal-rel=" & CHR$(34) & "paragraph" & CHR$(34)
            StyleBodyA = GetBodiesItemStyleName (gOfficePath & ConvertPath("user\work\xml\writer\level1\frames\content.xml") , "draw:text-box" , 1)
            printlog " fr1 = " & StyleBodyA
            printlog " =========="
            StyleDefinitionA = GetXMLValueLineExtra ((gOfficePath & ConvertPath("user\work\xml\writer\level1\frames\content.xml")), "office:document-content", "office:automatic-styles;style:style" , "style:name" , StyleBodyA)
            For i = 1 to 8
                printlog " +- " & Searchstring(i)
                if InStr (StyleDefinitionA , Searchstring(i)) = 0 then
                    warnlog "Frame Style: " & Searchstring(i) & " not found!"
                end if
            Next i            
            Searchstring(1) = "style:family=" & CHR$(34) & "graphics" & CHR$(34)
            Searchstring(2) = "style:parent-style-name=" & CHR$(34)
            Searchstring(3) = "style:run-through=" & CHR$(34) & "foreground" & CHR$(34)
            Searchstring(4) = "style:wrap=" & CHR$(34) & "none" & CHR$(34)
            Searchstring(5) = "style:vertical-pos=" & CHR$(34) & "from-top" & CHR$(34)
            Searchstring(6) = "style:vertical-rel=" & CHR$(34) & "page" & CHR$(34)
            Searchstring(7) = "style:horizontal-pos=" & CHR$(34) & "from-left" & CHR$(34)
            Searchstring(8) = "style:horizontal-rel=" & CHR$(34) & "paragraph" & CHR$(34)
            StyleBodyA = GetBodiesItemStyleName (gOfficePath & ConvertPath("user\work\xml\writer\level1\frames\content.xml") , "draw:text-box" , 2)
            printlog " fr2 = " & StyleBodyA
            printlog " =========="
            StyleDefinitionA = GetXMLValueLineExtra ((gOfficePath & ConvertPath("user\work\xml\writer\level1\frames\content.xml")), "office:document-content", "office:automatic-styles;style:style" , "style:name" , StyleBodyA)
            For i = 1 to 8
                printlog " +- " & Searchstring(i)
                if InStr (StyleDefinitionA , Searchstring(i)) = 0 then
                    warnlog "Frame Style: " & Searchstring(i) & " not found!"
                end if
            Next i
            Searchstring(1) = "style:family=" & CHR$(34) & "graphics" & CHR$(34)
            Searchstring(2) = "style:parent-style-name=" & CHR$(34)
            Searchstring(3) = "style:wrap=" & CHR$(34) & "parallel" & CHR$(34)
            Searchstring(4) = "style:run-through=" & CHR$(34) & "foreground" & CHR$(34)
            Searchstring(5) = "style:number-wrapped-paragraphs=" & CHR$(34) & "no-limit" & CHR$(34)
            Searchstring(6) = "style:vertical-pos=" & CHR$(34) & "from-top" & CHR$(34)
            Searchstring(7) = "style:vertical-rel=" & CHR$(34) & "page" & CHR$(34)
            Searchstring(8) = "style:horizontal-pos=" & CHR$(34) & "from-left" & CHR$(34)
            Searchstring(9) = "style:horizontal-rel=" & CHR$(34) & "paragraph" & CHR$(34)
            StyleBodyA = GetBodiesItemStyleName (gOfficePath & ConvertPath("user\work\xml\writer\level1\frames\content.xml") , "draw:text-box" , 3)
            printlog " fr3 = " & StyleBodyA
            printlog " =========="
            StyleDefinitionA = GetXMLValueLineExtra ((gOfficePath & ConvertPath("user\work\xml\writer\level1\frames\content.xml")), "office:document-content", "office:automatic-styles;style:style" , "style:name" , StyleBodyA)
            For i = 1 to 9
                printlog " +- " & Searchstring(i)
                if InStr (StyleDefinitionA , Searchstring(i)) = 0 then
                    warnlog "Frame Style: " & Searchstring(i) & " not found!"
                end if
            Next i            
            Searchstring(1) = "style:family=" & CHR$(34) & "graphics" & CHR$(34)
            Searchstring(2) = "style:parent-style-name=" & CHR$(34)
            Searchstring(3) = "style:wrap=" & CHR$(34) & "dynamic" & CHR$(34)
            Searchstring(4) = "style:run-through=" & CHR$(34) & "foreground" & CHR$(34)
            Searchstring(5) = "style:number-wrapped-paragraphs=" & CHR$(34) & "no-limit" & CHR$(34)
            Searchstring(6) = "style:vertical-pos=" & CHR$(34) & "from-top" & CHR$(34)
            Searchstring(7) = "style:vertical-rel=" & CHR$(34) & "page" & CHR$(34)
            Searchstring(8) = "style:horizontal-pos=" & CHR$(34) & "from-left" & CHR$(34)
            Searchstring(9) = "style:horizontal-rel=" & CHR$(34) & "paragraph" & CHR$(34)
            StyleBodyA = GetBodiesItemStyleName (gOfficePath & ConvertPath("user\work\xml\writer\level1\frames\content.xml") , "draw:text-box" , 4)
            printlog " fr4 = " & StyleBodyA
            printlog " =========="
            StyleDefinitionA = GetXMLValueLineExtra ((gOfficePath & ConvertPath("user\work\xml\writer\level1\frames\content.xml")), "office:document-content", "office:automatic-styles;style:style" , "style:name" , StyleBodyA)
            For i = 1 to 9
                printlog " +- " & Searchstring(i)
                if InStr (StyleDefinitionA , Searchstring(i)) = 0 then
                    warnlog "Frame Style: " & Searchstring(i) & " not found!"
                end if
            Next i
            Searchstring(1) = "style:family=" & CHR$(34) & "graphics" & CHR$(34)
            Searchstring(2) = "style:parent-style-name=" & CHR$(34)
            Searchstring(3) = "style:wrap=" & CHR$(34) & "run-through" & CHR$(34)
            Searchstring(4) = "style:run-through=" & CHR$(34) & "foreground" & CHR$(34)
            Searchstring(5) = "style:vertical-pos=" & CHR$(34) & "from-top" & CHR$(34)
            Searchstring(6) = "style:vertical-rel=" & CHR$(34) & "page" & CHR$(34)
            Searchstring(7) = "style:horizontal-pos=" & CHR$(34) & "from-left" & CHR$(34)
            Searchstring(8) = "style:horizontal-rel=" & CHR$(34) & "page" & CHR$(34)
            StyleBodyA = GetBodiesItemStyleName (gOfficePath & ConvertPath("user\work\xml\writer\level1\frames\content.xml") , "draw:text-box" , 5)
            printlog " fr5 = " & StyleBodyA
            printlog " =========="
            StyleDefinitionA = GetXMLValueLineExtra ((gOfficePath & ConvertPath("user\work\xml\writer\level1\frames\content.xml")), "office:document-content", "office:automatic-styles;style:style" , "style:name" , StyleBodyA)
            For i = 1 to 8
                printlog " +- " & Searchstring(i)
                if InStr (StyleDefinitionA , Searchstring(i)) = 0 then
                    warnlog "Frame Style: " & Searchstring(i) & " not found!"
                end if
            Next i
            Searchstring(1) = "style:family=" & CHR$(34) & "graphics" & CHR$(34)
            Searchstring(2) = "style:parent-style-name=" & CHR$(34)
            Searchstring(3) = "style:wrap=" & CHR$(34) & "run-through" & CHR$(34)
            Searchstring(4) = "style:run-through=" & CHR$(34) & "background" & CHR$(34)
            Searchstring(5) = "style:vertical-pos=" & CHR$(34) & "from-top" & CHR$(34)
            Searchstring(6) = "style:vertical-rel=" & CHR$(34) & "page" & CHR$(34)
            Searchstring(7) = "style:horizontal-pos=" & CHR$(34) & "from-left" & CHR$(34)
            Searchstring(8) = "style:horizontal-rel=" & CHR$(34) & "paragraph" & CHR$(34)            
            StyleBodyA = GetBodiesItemStyleName (gOfficePath & ConvertPath("user\work\xml\writer\level1\frames\content.xml") , "draw:text-box" , 6)
            printlog " fr6 = " & StyleBodyA
            printlog " =========="
            StyleDefinitionA = GetXMLValueLineExtra ((gOfficePath & ConvertPath("user\work\xml\writer\level1\frames\content.xml")), "office:document-content", "office:automatic-styles;style:style" , "style:name" , StyleBodyA)
            For i = 1 to 8
                printlog " +- " & Searchstring(i)
                if InStr (StyleDefinitionA , Searchstring(i)) = 0 then
                    warnlog "Frame Style: " & Searchstring(i) & " not found!"
                end if
            Next i
            Searchstring(1) = "style:family=" & CHR$(34) & "graphics" & CHR$(34)
            Searchstring(2) = "style:parent-style-name=" & CHR$(34)
            Searchstring(3) = "style:wrap=" & CHR$(34) & "parallel" & CHR$(34)
            Searchstring(4) = "style:number-wrapped-paragraphs=" & CHR$(34) & "no-limit" & CHR$(34)
            Searchstring(5) = "style:vertical-pos=" & CHR$(34) & "from-top" & CHR$(34)
            Searchstring(6) = "style:vertical-rel=" & CHR$(34) & "paragraph" & CHR$(34)
            Searchstring(7) = "style:horizontal-pos=" & CHR$(34) & "from-left" & CHR$(34)
            Searchstring(8) = "style:horizontal-rel=" & CHR$(34) & "paragraph" & CHR$(34)
            StyleBodyA = GetBodiesItemStyleName (gOfficePath & ConvertPath("user\work\xml\writer\level1\frames\content.xml") , "draw:text-box", 7)
            printlog " fr7 = " & StyleBodyA
            printlog " =========="
            StyleDefinitionA = GetXMLValueLineExtra ((gOfficePath & ConvertPath("user\work\xml\writer\level1\frames\content.xml")), "office:document-content", "office:automatic-styles;style:style" , "style:name" , StyleBodyA)
            For i = 1 to 8
                printlog " +- " & Searchstring(i)
                if InStr (StyleDefinitionA , Searchstring(i)) = 0 then
                    warnlog "Frame Style: " & Searchstring(i) & " not found!"
                end if
            Next i
            Searchstring(1) = "style:family=" & CHR$(34) & "graphics" & CHR$(34)
            Searchstring(2) = "style:parent-style-name=" & CHR$(34)
            Searchstring(3) = "style:wrap=" & CHR$(34) & "parallel" & CHR$(34)
            Searchstring(4) = "style:number-wrapped-paragraphs=" & CHR$(34) & "no-limit" & CHR$(34)
            Searchstring(5) = "style:vertical-pos=" & CHR$(34) & "from-top" & CHR$(34)
            Searchstring(6) = "style:horizontal-pos=" & CHR$(34) & "from-left" & CHR$(34)
            Searchstring(7) = "style:horizontal-rel=" & CHR$(34) & "paragraph" & CHR$(34)
            StyleBodyA = GetBodiesItemStyleName (gOfficePath & ConvertPath("user\work\xml\writer\level1\frames\content.xml") , "draw:text-box", 8)
            printlog " fr8 = " & StyleBodyA
            printlog " =========="
            StyleDefinitionA = GetXMLValueLineExtra ((gOfficePath & ConvertPath("user\work\xml\writer\level1\frames\content.xml")), "office:document-content", "office:automatic-styles;style:style" , "style:name" , StyleBodyA)
            For i = 1 to 7
                printlog " +- " & Searchstring(i)
                if InStr (StyleDefinitionA , Searchstring(i)) = 0 then
                    warnlog "Frame Style: " & Searchstring(i) & " not found!"
                end if
            Next i
            Searchstring(1) = "style:family=" & CHR$(34) & "graphics" & CHR$(34)
            Searchstring(2) = "style:parent-style-name=" & CHR$(34)
            Searchstring(3) = "style:wrap=" & CHR$(34) & "parallel" & CHR$(34)
            Searchstring(4) = "style:number-wrapped-paragraphs=" & CHR$(34) & "no-limit" & CHR$(34)
            Searchstring(5) = "style:vertical-pos=" & CHR$(34) & "from-top" & CHR$(34)
            Searchstring(6) = "style:horizontal-pos=" & CHR$(34) & "from-left" & CHR$(34)
            Searchstring(7) = "style:horizontal-rel=" & CHR$(34) & "paragraph" & CHR$(34)            
            StyleBodyA = GetBodiesItemStyleName (gOfficePath & ConvertPath("user\work\xml\writer\level1\frames\content.xml") , "draw:text-box", 9)
            printlog " fr9 = " & StyleBodyA
            printlog " =========="
            StyleDefinitionA = GetXMLValueLineExtra ((gOfficePath & ConvertPath("user\work\xml\writer\level1\frames\content.xml")), "office:document-content", "office:automatic-styles;style:style" , "style:name" , StyleBodyA)
            For i = 1 to 7
                printlog " +- " & Searchstring(i)
                if InStr (StyleDefinitionA , Searchstring(i)) = 0 then
                    warnlog "Frame Style: " & Searchstring(i) & " not found!"
                end if
            Next i
            Searchstring(1) = "style:family=" & CHR$(34) & "graphics" & CHR$(34)
            Searchstring(2) = "style:parent-style-name=" & CHR$(34)
            Searchstring(3) = "style:protect=" & CHR$(34) & "content size position" & CHR$(34)
            Searchstring(4) = "style:wrap=" & CHR$(34) & "parallel" & CHR$(34)
            Searchstring(5) = "style:number-wrapped-paragraphs=" & CHR$(34) & "no-limit" & CHR$(34)
            Searchstring(6) = "style:vertical-pos=" & CHR$(34) & "from-top" & CHR$(34)
            Searchstring(7) = "style:vertical-rel=" & CHR$(34) & "paragraph" & CHR$(34)
            Searchstring(8) = "style:horizontal-pos=" & CHR$(34) & "from-left" & CHR$(34)
            Searchstring(9) = "style:horizontal-rel=" & CHR$(34) & "paragraph" & CHR$(34)
            StyleBodyA = GetBodiesItemStyleName (gOfficePath & ConvertPath("user\work\xml\writer\level1\frames\content.xml") , "draw:text-box", 10)
            printlog " fr10 = " & StyleBodyA
            printlog " =========="
            StyleDefinitionA = GetXMLValueLineExtra ((gOfficePath & ConvertPath("user\work\xml\writer\level1\frames\content.xml")), "office:document-content", "office:automatic-styles;style:style" , "style:name" , StyleBodyA)
            For i = 1 to 9
                printlog " +- " & Searchstring(i)
                if InStr (StyleDefinitionA , Searchstring(i)) = 0 then
                    warnlog "Frame Style: " & Searchstring(i) & " not found!"
                end if
            Next i
            Searchstring(1)  = "style:family=" & CHR$(34) & "graphics" & CHR$(34)
            Searchstring(2)  = "style:parent-style-name=" & CHR$(34)
            Searchstring(3)  = "style:wrap=" & CHR$(34) & "parallel" & CHR$(34)
            Searchstring(4)  = "style:number-wrapped-paragraphs=" & CHR$(34) & "no-limit" & CHR$(34)
            Searchstring(5)  = "style:vertical-pos=" & CHR$(34) & "from-top" & CHR$(34)
            Searchstring(6)  = "style:horizontal-pos=" & CHR$(34) & "from-left" & CHR$(34)
            Searchstring(7)  = "style:horizontal-rel=" & CHR$(34) & "paragraph" & CHR$(34)
            Searchstring(8)  = "style:border-line-width-left=" & CHR$(34)
            Searchstring(9)  = "style:border-line-width-bottom=" & CHR$(34)
            Searchstring(10) = "fo:padding-left=" & CHR$(34)
            Searchstring(11) = "fo:padding-right=" & CHR$(34)
            Searchstring(12) = "fo:padding-top=" & CHR$(34)
            Searchstring(13) = "fo:padding-bottom=" & CHR$(34)
            Searchstring(14) = "fo:border-left=" & CHR$(34)
            Searchstring(15) = "double"
            Searchstring(16) = "#0000ff"
            Searchstring(17) = "fo:border-bottom=" & CHR$(34)
            Searchstring(18) = "style:shadow=" & CHR$(34) & "#ff0000"
            StyleBodyA = GetBodiesItemStyleName (gOfficePath & ConvertPath("user\work\xml\writer\level1\frames\content.xml") , "draw:text-box", 11)
            printlog " fr11 = " & StyleBodyA
            printlog " =========="
            StyleDefinitionA = GetXMLValueLineExtra ((gOfficePath & ConvertPath("user\work\xml\writer\level1\frames\content.xml")), "office:document-content", "office:automatic-styles;style:style" , "style:name" , StyleBodyA)
            For i = 1 to 18
                printlog " +- " & Searchstring(i)
                if InStr (StyleDefinitionA , Searchstring(i)) = 0 then
                    warnlog "Frame Style: " & Searchstring(i) & " not found!"
                end if
            Next i
            Searchstring(1) = "style:family=" & CHR$(34) & "graphics" & CHR$(34)
            Searchstring(2) = "style:parent-style-name=" & CHR$(34)
            Searchstring(3) = "style:wrap=" & CHR$(34) & "parallel" & CHR$(34)
            Searchstring(4) = "style:number-wrapped-paragraphs=" & CHR$(34) & "no-limit" & CHR$(34)
            Searchstring(5) = "style:vertical-pos=" & CHR$(34) & "from-top" & CHR$(34)
            Searchstring(6) = "style:vertical-rel=" & CHR$(34) & "paragraph" & CHR$(34)
            Searchstring(7) = "style:horizontal-pos=" & CHR$(34) & "from-left" & CHR$(34)
            Searchstring(8) = "style:horizontal-rel=" & CHR$(34) & "paragraph" & CHR$(34)
            Searchstring(9) = "fo:background-color=" & CHR$(34) & "#ffff00" & CHR$(34)
            StyleBodyA = GetBodiesItemStyleName (gOfficePath & ConvertPath("user\work\xml\writer\level1\frames\content.xml") , "draw:text-box", 12)
            printlog " fr12 = " & StyleBodyA
            printlog " =========="
            StyleDefinitionA = GetXMLValueLineExtra ((gOfficePath & ConvertPath("user\work\xml\writer\level1\frames\content.xml")), "office:document-content", "office:automatic-styles;style:style" , "style:name" , StyleBodyA)
            For i = 1 to 9
                printlog " +- " & Searchstring(i)
                if InStr (StyleDefinitionA , Searchstring(i)) = 0 then
                    warnlog "Frame Style: " & Searchstring(i) & " not found!"
                end if
            Next i            
            Searchstring(1)  = "style:family=" & CHR$(34) & "graphics" & CHR$(34)
            Searchstring(2)  = "style:parent-style-name=" & CHR$(34)
            Searchstring(3)  = "style:wrap=" & CHR$(34) & "parallel" & CHR$(34)
            Searchstring(4)  = "style:number-wrapped-paragraphs=" & CHR$(34) & "no-limit" & CHR$(34)
            Searchstring(5)  = "style:vertical-pos=" & CHR$(34) & "from-top" & CHR$(34)
            Searchstring(6)  = "style:vertical-rel=" & CHR$(34) & "paragraph" & CHR$(34)
            Searchstring(7)  = "style:horizontal-pos=" & CHR$(34) & "from-left" & CHR$(34)
            Searchstring(8)  = "style:horizontal-rel=" & CHR$(34) & "paragraph" & CHR$(34)
            Searchstring(9)  = "fo:background-color=" & CHR$(34) & "transparent" & CHR$(34)
            Searchstring(10) = "<style:background-image/>"
            Searchstring(11) = "<style:columns fo:column-count=" & CHR$(34) & "2" & CHR$(34)
            
            Searchstring(13) = "fo:margin-left=" & CHR$(34)
            Searchstring(14) = "</style:columns>"
            Searchstring(15) = "<style:properties"
            Searchstring(16) = "</style:properties>"            
            StyleBodyA = GetBodiesItemStyleName (gOfficePath & ConvertPath("user\work\xml\writer\level1\frames\content.xml") , "draw:text-box", 13)
            printlog " fr13 = " & StyleBodyA
            printlog " =========="
            StyleDefinitionA = GetXMLValueLineExtra ((gOfficePath & ConvertPath("user\work\xml\writer\level1\frames\content.xml")), "office:document-content", "office:automatic-styles;style:style" , "style:name" , StyleBodyA)
            For i = 1 to 16
                if i <> 12 then
                    printlog " +- " & Searchstring(i)
                    if InStr (StyleDefinitionA , Searchstring(i)) = 0 then
                        warnlog "Frame Style: " & Searchstring(i) & " not found!"
                    end if
                end if
            Next i
        end if
    else
        call hCloseDocument
    end if    
endcase
'
'-------------------------------------------------------------------------
'
testcase tschina
    Dim i as integer
    Dim a as integer
    Dim xXMLStyleName as string
    Dim xPath as string
    Dim AttributeSearch(50) as string
    Dim AttributeValue(50) as string
    printlog "+- zh_cn_text.sxw"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\writer\zh_cn_text.sxw"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\writer\level1\zh_cn_text.sxw") , "StarOffice XML (Writer)") then
        '/// Closing the document also if there is a verification dialog.
        call hCloseDocument()
        sleep(3)     
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\writer\level1\zh_cn_text.sxw") , gOfficePath & ConvertPath("user\work\xml\writer\level1\zh_cn_text") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\writer\level1\zh_cn_text\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\writer\level1\zh_cn_text\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\writer\level1\zh_cn_text\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\writer\level1\zh_cn_text.sxw"))
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
testcase ttchina
    Dim i as integer
    Dim a as integer
    Dim xXMLStyleName as string
    Dim xPath as string
    Dim AttributeSearch(50) as string
    Dim AttributeValue(50) as string
    printlog "+- zh_tw_text.sxw"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\writer\zh_tw_text.sxw"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\writer\level1\zh_tw_text.sxw") , "StarOffice XML (Writer)") then
        '/// Closing the document also if there is a verification dialog.
        call hCloseDocument()
        sleep(3)     
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\writer\level1\zh_tw_text.sxw") , gOfficePath & ConvertPath("user\work\xml\writer\level1\zh_tw_text") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\writer\level1\zh_tw_text\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\writer\level1\zh_tw_text\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\writer\level1\zh_tw_text\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\writer\level1\zh_tw_text.sxw"))
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
testcase tkorean
    Dim i as integer
    Dim a as integer
    Dim xXMLStyleName as string
    Dim xPath as string
    Dim AttributeSearch(50) as string
    Dim AttributeValue(50) as string
    printlog "+- ko_text.sxw"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\writer\ko_text.sxw"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\writer\level1\ko_text.sxw") , "StarOffice XML (Writer)") then
        '/// Closing the document also if there is a verification dialog.
        call hCloseDocument()
        sleep(3)     
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\writer\level1\ko_text.sxw") , gOfficePath & ConvertPath("user\work\xml\writer\level1\ko_text") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\writer\level1\ko_text\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\writer\level1\ko_text\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\writer\level1\ko_text\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\writer\level1\ko_text.sxw"))
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
testcase tjapan
    Dim i as integer
    Dim a as integer
    Dim xXMLStyleName as string
    Dim xPath as string
    Dim AttributeSearch(50) as string
    Dim AttributeValue(50) as string
    printlog "+- jp_text.sxw"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\writer\jp_text.sxw"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\writer\level1\jp_text.sxw") , "StarOffice XML (Writer)") then
        '/// Closing the document also if there is a verification dialog.
        call hCloseDocument()
        sleep(3)     
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\writer\level1\jp_text.sxw") , gOfficePath & ConvertPath("user\work\xml\writer\level1\jp_text") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\writer\level1\jp_text\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\writer\level1\jp_text\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\writer\level1\jp_text\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\writer\level1\jp_text.sxw"))
            sleep(2)
            call hCloseDocument
            sleep(2)
        end if
    else
        call hCloseDocument
    end if
endcase

