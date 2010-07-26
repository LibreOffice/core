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
   Dim iSecurityBefore as integer

sub sxw7_03

    printlog "-------------------------- SXW7_03.INC ------------------------------"
    call hEnablePrettyPrinting
    call thyperlinks
    call hEnablePrettyPrinting(1)
    call tindexes_alpha
    call tindex_biblio
    call tindex_most
    call tpages
    call tparagraph01
    call tparagraph02
    call tsection_main
    printlog "------------------------ Macros and Events --------------------------"
        iSecurityBefore = ""
        iSecurityBefore = hSetMacroSecurity(1)   
    call tEventsToObjects
        call hSetMacroSecurity(iSecurityBefore)
end sub

'-------------------------------------------------------------------------

testcase thyperlinks
    Dim Searchstring(9) as string
    Dim i as integer
    Dim AdditionalParameter as string
    printlog "+- hyperlinks.sdw"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\writer\hyperlinks.sdw"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\writer\level1\hyperlinks.sxw") , "StarOffice XML (Writer)") then
        '/// Closing the document also if there is a verification dialog.
        call hCloseDocument()
        sleep(3)     
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\writer\level1\hyperlinks.sxw") , gOfficePath & ConvertPath("user\work\xml\writer\level1\hyperlinks") )
        if XMLWellFormed (gOfficePath & ConvertPath("user\work\xml\writer\level1\hyperlinks\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellFormed (gOfficePath & ConvertPath("user\work\xml\writer\level1\hyperlinks\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellFormed (gOfficePath & ConvertPath("user\work\xml\writer\level1\hyperlinks\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\writer\level1\hyperlinks.sxw"))
            sleep(2)
            call hCloseDocument
            sleep(2)
            '    -----------------------
            '     T E X T - S T Y L E S
            '    -----------------------
            '    <text:p text:style-name="Standard"><text:a xlink:type="simple" xlink:href="http://Www.sun.com/" office:target-frame-name="_blank" xlink:show="new" text:style-name="Variable" text:visited-style-name="Quotation"><text:span text:style-name="T1">Www.sun.com</text:span></text:a></text:p>
            '-------------------------------------------------------------------------
            Searchstring(1) = "<text:a"
            Searchstring(2) = "xlink:type=" & CHR$(34) & "simple" & CHR$(34)
            Searchstring(3) = "xlink:href=" & CHR$(34) & "http://Www.sun.com/" & CHR$(34)
            Searchstring(4) = "office:target-frame-name=" & CHR$(34) & "_blank" & CHR$(34)
            Searchstring(5) = "xlink:show=" & CHR$(34) & "new" & CHR$(34)
            Searchstring(6) = "text:visited-style-name=" & CHR$(34)
            Searchstring(7) = ">Www.sun.com<"
            Searchstring(8) = "</text:span>"
            Searchstring(9) = "</text:a>"
            '-------------------------------------------------------------------------
            AdditionalParameter = GetLineInXMLBody(gOfficePath & ConvertPath("user\work\xml\writer\level1\hyperlinks\content.xml") , "text:p" , 1)
            printlog "Debug: " & AdditionalParameter
            For i = 1 to 9
                printlog " +- " & Searchstring(i)
                if InStr (AdditionalParameter , Searchstring(i)) = 0 then
                    warnlog "Hyperlinks in text: " & Searchstring(i) & " not found!"
                end if
            Next i
            '   <text:p text:style-name="P1"><text:a xlink:type="simple" xlink:href="#targ" text:style-name="Variable" text:visited-style-name="Quotation">jump</text:a></text:p>
            '-------------------------------------------------------------------------
            Searchstring(1) = "<text:a"
            Searchstring(2) = "xlink:type=" & CHR$(34) & "simple" & CHR$(34)
            Searchstring(3) = "xlink:href=" & CHR$(34) & "#targ" & CHR$(34)
            Searchstring(4) = "text:visited-style-name=" & CHR$(34)
            Searchstring(5) = ">jump<"
            Searchstring(6) = "</text:a>"
            '-------------------------------------------------------------------------
            AdditionalParameter = GetLineInXMLBody(gOfficePath & ConvertPath("user\work\xml\writer\level1\hyperlinks\content.xml") , "text:p" , 3)
            For i = 1 to 6
                printlog " +- " & Searchstring(i)
                if InStr (AdditionalParameter , Searchstring(i)) = 0 then
                    warnlog "Hyperlinks in text: " & Searchstring(i) & " not found!"
                end if
            Next i
            '   <text:p text:style-name="P1"><text:bookmark-start text:name="targ"/>target here<text:bookmark-end text:name="targ"/></text:p>
            '-------------------------------------------------------------------------
            Searchstring(1) = "<text:bookmark-start"
            Searchstring(2) = "text:name=" & CHR$(34) & "targ" & CHR$(34)
            Searchstring(3) = ">target here<"
            Searchstring(4) = "<text:bookmark-end"
            '-------------------------------------------------------------------------
            AdditionalParameter = GetLineInXMLBody(gOfficePath & ConvertPath("user\work\xml\writer\level1\hyperlinks\content.xml") , "text:p" , 6)
            For i = 1 to 4
                printlog " +- " & Searchstring(i)
                if InStr (AdditionalParameter , Searchstring(i)) = 0 then
                    warnlog "Hyperlinks in text: " & Searchstring(i) & " not found!"
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
testcase tindexes_alpha
    printlog "+- indexes_alpha.sdw"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\writer\indexes_alpha.sdw"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\writer\level1\indexes_alpha.sxw") , "StarOffice XML (Writer)") then
        '/// Closing the document also if there is a verification dialog.
        call hCloseDocument()
        sleep(3)     
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\writer\level1\indexes_alpha.sxw") , gOfficePath & ConvertPath("user\work\xml\writer\level1\indexes_alpha") )
        if XMLWellFormed (gOfficePath & ConvertPath("user\work\xml\writer\level1\indexes_alpha\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellFormed (gOfficePath & ConvertPath("user\work\xml\writer\level1\indexes_alpha\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellFormed (gOfficePath & ConvertPath("user\work\xml\writer\level1\indexes_alpha\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\writer\level1\indexes_alpha.sxw"))
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
testcase tindex_biblio
    printlog "+- index_biblio.sdw"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\writer\index_biblio.sdw"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\writer\level1\index_biblio.sxw") , "StarOffice XML (Writer)") then
        '/// Closing the document also if there is a verification dialog.
        call hCloseDocument()
        sleep(3)     
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\writer\level1\index_biblio.sxw") , gOfficePath & ConvertPath("user\work\xml\writer\level1\index_biblio") )
        if XMLWellFormed (gOfficePath & ConvertPath("user\work\xml\writer\level1\index_biblio\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellFormed (gOfficePath & ConvertPath("user\work\xml\writer\level1\index_biblio\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellFormed (gOfficePath & ConvertPath("user\work\xml\writer\level1\index_biblio\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\writer\level1\index_biblio.sxw"))
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
testcase tindex_most
    printlog "+- index_most.sdw"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\writer\index_most.sdw"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\writer\level1\index_most.sxw") , "StarOffice XML (Writer)") then
        '/// Closing the document also if there is a verification dialog.
        call hCloseDocument()
        sleep(3)     
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\writer\level1\index_most.sxw") , gOfficePath & ConvertPath("user\work\xml\writer\level1\index_most") )
        if XMLWellFormed (gOfficePath & ConvertPath("user\work\xml\writer\level1\index_most\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellFormed (gOfficePath & ConvertPath("user\work\xml\writer\level1\index_most\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellFormed (gOfficePath & ConvertPath("user\work\xml\writer\level1\index_most\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\writer\level1\index_most.sxw"))
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
testcase tpages
    printlog "+- pages.sdw"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\writer\pages.sdw"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\writer\level1\pages.sxw") , "StarOffice XML (Writer)") then
        '/// Closing the document also if there is a verification dialog.
        call hCloseDocument()
        sleep(3)     
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\writer\level1\pages.sxw") , gOfficePath & ConvertPath("user\work\xml\writer\level1\pages") )
        if XMLWellFormed (gOfficePath & ConvertPath("user\work\xml\writer\level1\pages\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellFormed (gOfficePath & ConvertPath("user\work\xml\writer\level1\pages\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellFormed (gOfficePath & ConvertPath("user\work\xml\writer\level1\pages\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\writer\level1\pages.sxw"))
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
testcase tparagraph01
    printlog "+- paragraph01.sdw"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\writer\paragraph01.sdw"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\writer\level1\paragraph01.sxw") , "StarOffice XML (Writer)") then
        '/// Closing the document also if there is a verification dialog.
        call hCloseDocument()
        sleep(3)     
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\writer\level1\paragraph01.sxw") , gOfficePath & ConvertPath("user\work\xml\writer\level1\paragraph01") )
        if XMLWellFormed (gOfficePath & ConvertPath("user\work\xml\writer\level1\paragraph01\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellFormed (gOfficePath & ConvertPath("user\work\xml\writer\level1\paragraph01\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellFormed (gOfficePath & ConvertPath("user\work\xml\writer\level1\paragraph01\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\writer\level1\paragraph01.sxw"))
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
testcase tparagraph02
    printlog "+- paragraph02.sdw"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\writer\paragraph02.sdw"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\writer\level1\paragraph02.sxw") , "StarOffice XML (Writer)") then
        '/// Closing the document also if there is a verification dialog.
        call hCloseDocument()
        sleep(3)     
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\writer\level1\paragraph02.sxw") , gOfficePath & ConvertPath("user\work\xml\writer\level1\paragraph02") )
        if XMLWellFormed (gOfficePath & ConvertPath("user\work\xml\writer\level1\paragraph02\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellFormed (gOfficePath & ConvertPath("user\work\xml\writer\level1\paragraph02\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellFormed (gOfficePath & ConvertPath("user\work\xml\writer\level1\paragraph02\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\writer\level1\paragraph02.sxw"))
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
testcase tsection_main
    QAErrorLog "#i88812#-Update all links message doubled in old .sd* documents."
    goto endsub
    printlog "+- section_main.sdw"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\writer\section_main.sdw") , TRUE)
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\writer\level1\section_main.sxw") , "StarOffice XML (Writer)") then
        '/// Closing the document also if there is a verification dialog.
        call hCloseDocument()
        sleep(3)     
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\writer\level1\section_main.sxw") , gOfficePath & ConvertPath("user\work\xml\writer\level1\section_main") )
        if XMLWellFormed (gOfficePath & ConvertPath("user\work\xml\writer\level1\section_main\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellFormed (gOfficePath & ConvertPath("user\work\xml\writer\level1\section_main\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellFormed (gOfficePath & ConvertPath("user\work\xml\writer\level1\section_main\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\writer\level1\section_main.sxw") , TRUE)
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
testcase tEventsToObjects
    '/// Goal: All macros embedded to the objects should be exported corrextly to XML fileformat. 
    Dim i as integer
    Dim l as integer
    Dim a as integer
    Dim xXMLStyleName as string
    Dim xPath as string
    Dim AttributeSearch(50) as string
    Dim AttributeValue(50) as string
    Dim cWhereCalled as string    
    
    dim sdw_file as string 
        sdw_file = gTesttoolPath & "xml\optional\input\writer\events_to_objects.sdw"
        sdw_file = convertpath( sdw_file )
    dim sxw_file as string 
        sxw_file = gOfficePath & "user\work\xml\writer\level1\events_to_objects.sxw"
        sxw_file = convertpath( sxw_file )
    
    printlog "+- events_to_objects.sdw"

    cWhereCalled = "xml::level1::inc::sxw7_03.inc::tEventsToObjects: "
    
    '/// Open Tools/Options -> OpenOffice.org/Security
    '/// Click on the macro security button
    '/// Set the <b>medium</b> security level
    
    try
        if iSecurityBefore = "" then
            QAErrorLog cWhereCalled & "Macro security level needs to be set before to 1 (medium)!"
            goto endsub
        end if
    catch
        'if variable iSecurityBefore is not initialized and causes into an error
        QAErrorLog cWhereCalled & "Macro security level needs to be set before to 1 (medium)!"
        goto endsub        
    endcatch        
  
    '/// Load (binary) document under <i>...qa/qatesttool/xml/update/input/writer/events_to_objects.sdw</i>.
    hFileOpen( sdw_file )
    if ( NOT hAllowMacroExecution() ) then
        warnlog "#i80769# Macros not being loaded. Exiting test case."
        call hCloseDocument
        goto endsub
    end if    
    '/// Save as StarOffice XML fileformat (.sxw).  
    if ( hFileSaveAsWithFilterKill ( sxw_file , "StarOffice XML (Writer)" ) ) then    
        '/// Closing the document also if there is a verification dialog.
        'call hCloseDocument()
        hCloseDocument()
        'sleep(3)     
        '/// Close window with the document.
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\writer\level1\events_to_objects.sxw") , _
                       gOfficePath & ConvertPath("user\work\xml\writer\level1\events_to_objects") )
        '/// Uncompressing .sxw files in directory <pre>../user/work/events_to_objects/</pre>. 
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\writer\level1\events_to_objects\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        '///+Verify that XML files (<i>styles.xml</i>, <i>meta.xml</i> and <i>content.xml</i>) are <i>well formed</i>. 
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\writer\level1\events_to_objects\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\writer\level1\events_to_objects\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
            '/// If <i>well formness</i> is okay, load exported document.
        else
            hFileOpen( sdw_file )
            hAllowMacroExecution()

            Kontext "DocumentWriter"
            DocumentWriter.TypeKeys "<SHIFT F4>" , TRUE
            '/// Select first graphic with &lt;SHIFT&gt;&lt;F4&gt;
            '///+ and a messagebox should be displayed.
            '///+ Press &lt;RETURN&gt; key.
            '///<i>four times</i>
            '///+<ol><li>Press &lt;TAB&gt; key to select the next object.</li><li>and a messagebox should be displayed.</li><li>Press &lt;RETURN&gt; key.</li></ol>
            for l = 1 to 4
                Kontext "Messagebox"
                if Messagebox.Exists(2) then
                    if Messagebox.GetRT = 304 then
                        Messagebox.OK
                    end if
                else
                    if l = 1 then 
                        warnlog "Event on Graphic failed! No Message Box occured!"
                    end if
                    if l = 2 then 
                        warnlog "Event on OLE object failed! No Message Box occured!"
                    end if
                    if l = 3 then 
                        warnlog "Event on text frame object failed! No Message Box occured!"
                    end if
                    if l = 4 then 
                        warnlog "Event on text frame object (frame style applied) failed! No Message Box occured!"
                    end if
                end if
                if l <> 4 then
                    Kontext "DocumentWriter"
                    DocumentWriter.TypeKeys "<TAB>"
                end if
            next l
            '/// Close document. 
            call hCloseDocument
            sleep(2)
            'TODO: Search for the events in source.       
        end if
    else
        call hCloseDocument
    end if
endcase

