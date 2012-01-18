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
'* short description : XML Calc Include File
'*
'\***********************************************************************************
   Dim Isliste(250) as string
   Dim OutputPath as string

sub sxc7_02

   printlog "------------------- sxc_02.inc ---------------------"
   printlog "---------------------- Cells -----------------------"

    call tcellformats2b
    call tcellformats5
    call tcellformats6

   printlog "---------------------- C J K -----------------------"

    call tjapan
    call tkorean
    call tsimple_c
    call trad_c

end sub

'-------------------------------------------------------------------------
testcase tcellformats2b
    printlog "+- cellformats2b"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\calc\so_binary\cellformats2b.sdc"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\calc\level1\cellformats2b.sxc") , "StarOffice XML (Calc)") then
        '/// Closing the document also if there is a verification dialog.
        call hCloseDocument()
        sleep(3)     
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\calc\level1\cellformats2b.sxc") , gOfficePath & ConvertPath("user\work\xml\calc\level1\cellformats2b") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\calc\level1\cellformats2b\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\calc\level1\cellformats2b\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\calc\level1\cellformats2b\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\calc\level1\cellformats2b.sxc"))
            sleep(2)
            call hCloseDocument
            sleep(2)
        end if
    else
        call hCloseDocument
    end if
endcase

'-------------------------------------------------------------------------
testcase tcellformats5
    printlog "+- cellformats5.sdc"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\calc\so_binary\cellformats5.sdc"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\calc\level1\cellformats5.sxc") , "StarOffice XML (Calc)") then
        '/// Closing the document also if there is a verification dialog.
        call hCloseDocument()
        sleep(3)     
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\calc\level1\cellformats5.sxc") , gOfficePath & ConvertPath("user\work\xml\calc\level1\cellformats5") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\calc\level1\cellformats5\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\calc\level1\cellformats5\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\calc\level1\cellformats5\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\calc\level1\cellformats5.sxc"))
            sleep(2)
            call hCloseDocument
            sleep(2)
        end if
    else
        call hCloseDocument
    end if
endcase

'-------------------------------------------------------------------------

testcase tcellformats6
    printlog "+- cellformats6.sdc"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\calc\so_binary\cellformats6.sdc"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\calc\level1\cellformats6.sxc") , "StarOffice XML (Calc)") then
        '/// Closing the document also if there is a verification dialog.
        call hCloseDocument()
        sleep(3)     
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\calc\level1\cellformats6.sxc") , gOfficePath & ConvertPath("user\work\xml\calc\level1\cellformats6") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\calc\level1\cellformats6\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\calc\level1\cellformats6\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\calc\level1\cellformats6\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\calc\level1\cellformats6.sxc"))
            sleep(2)
            call hCloseDocument
            sleep(2)
        end if
    else
        call hCloseDocument
    end if
endcase

'-------------------------------------------------------------------------

testcase tjapan
    printlog "+- japan.sxc"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\calc\ooo10\japan.sxc"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\calc\level1\japan.sxc") , "StarOffice XML (Calc)") then
        '/// Closing the document also if there is a verification dialog.
        call hCloseDocument()
        sleep(3)     
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\calc\level1\japan.sxc") , gOfficePath & ConvertPath("user\work\xml\calc\level1\japan") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\calc\level1\japan\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\calc\level1\japan\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\calc\level1\japan\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\calc\level1\japan.sxc"))
            sleep(2)
            call hCloseDocument
            sleep(2)
        end if
    else
        call hCloseDocument
    end if
endcase

'-------------------------------------------------------------------------

testcase tkorean
    printlog "+- korean.sxc"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\calc\ooo10\korean.sxc"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\calc\level1\korean.sxc") , "StarOffice XML (Calc)") then
        '/// Closing the document also if there is a verification dialog.
        call hCloseDocument()
        sleep(3)
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\calc\level1\korean.sxc") , gOfficePath & ConvertPath("user\work\xml\calc\level1\korean") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\calc\level1\korean\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\calc\level1\korean\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\calc\level1\korean\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\calc\level1\korean.sxc"))
            sleep(2)
            call hCloseDocument
            sleep(2)
        end if
    else
        call hCloseDocument
    end if
endcase

'-------------------------------------------------------------------------

testcase tsimple_c
    printlog "+- simple_c.sxc"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\calc\ooo10\simple_c.sxc"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\calc\level1\simple_c.sxc") , "StarOffice XML (Calc)") then
        '/// Closing the document also if there is a verification dialog.
        call hCloseDocument()
        sleep(3)
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\calc\level1\simple_c.sxc") , gOfficePath & ConvertPath("user\work\xml\calc\level1\simple_c") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\calc\level1\simple_c\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\calc\level1\simple_c\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\calc\level1\simple_c\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\calc\level1\simple_c.sxc"))
            sleep(2)
            call hCloseDocument
            sleep(2)
        end if
    else
        call hCloseDocument
    end if
endcase

'-------------------------------------------------------------------------

testcase trad_c
    printlog "+- trad_c.sxc"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\calc\ooo10\trad_c.sxc"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\calc\level1\trad_c.sxc") , "StarOffice XML (Calc)") then
        '/// Closing the document also if there is a verification dialog.
        call hCloseDocument()
        sleep(3)
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\calc\level1\trad_c.sxc") , gOfficePath & ConvertPath("user\work\xml\calc\level1\trad_c") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\calc\level1\trad_c\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\calc\level1\trad_c\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\calc\level1\trad_c\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\calc\level1\trad_c.sxc"))
            sleep(2)
            call hCloseDocument
            sleep(2)
        end if
    else
        call hCloseDocument
    end if
endcase

