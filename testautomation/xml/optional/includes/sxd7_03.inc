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

sub sxd7_03

   printlog "------------------- sxd_03.inc ---------------------"

   Call ExitRestartTheOffice
   call t18126
   call t18127_18128
   call t18129
   call t18130_18131
   call t18132
   call t18133_18134
   call t18135
   call t18136
   call t18137
   call t18138

end sub
'
'-------------------------------------------------------------------------
'
testcase t18126
    printlog "+- Draw: 18126.sda"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\graphics\so_bin\18126.sda"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\draw\level1\18126.sxd") , "StarOffice XML (Draw)") then
        call hCloseDocument
        sleep(2)
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\draw\level1\18126.sxd") , gOfficePath & ConvertPath("user\work\xml\draw\level1\18126\") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18126\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18126\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18126\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\draw\level1\18126.sxd")
            call hCloseDocument
        end if
    else
        call hCloseDocument
    end if
endcase
'
'-------------------------------------------------------------------------
'
testcase t18127_18128
    printlog "+- Draw: 18127-18128.sda"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\graphics\so_bin\18127-18128.sda"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\draw\level1\18127-18128.sxd") , "StarOffice XML (Draw)") then
        call hCloseDocument
        sleep(2)
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\draw\level1\18127-18128.sxd") , gOfficePath & ConvertPath("user\work\xml\draw\level1\18127-18128\") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18127-18128\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18127-18128\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18127-18128\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\draw\level1\18127-18128.sxd")
            call hCloseDocument
        end if
    else
        call hCloseDocument
    end if
endcase
'
'-------------------------------------------------------------------------
'
testcase t18129
    printlog "+- Draw: 18129.sda"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\graphics\so_bin\18129.sda"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\draw\level1\18129.sxd") , "StarOffice XML (Draw)") then
        call hCloseDocument
        sleep(2)
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\draw\level1\18129.sxd") , gOfficePath & ConvertPath("user\work\xml\draw\level1\18129\") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18129\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18129\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18129\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\draw\level1\18129.sxd")
            call hCloseDocument
        end if
    else
        call hCloseDocument
    end if
endcase
'
'-------------------------------------------------------------------------
'
testcase t18130_18131
    printlog "+- Draw: 18130-18131.sda"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\graphics\so_bin\18130-18131.sda"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\draw\level1\18130-18131.sxd") , "StarOffice XML (Draw)") then
        call hCloseDocument
        sleep(2)
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\draw\level1\18130-18131.sxd") , gOfficePath & ConvertPath("user\work\xml\draw\level1\18130-18131\") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18130-18131\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18130-18131\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18130-18131\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\draw\level1\18130-18131.sxd")
            call hCloseDocument
        end if
    else
        call hCloseDocument
    end if
endcase
'
'-------------------------------------------------------------------------
'
testcase t18132
    printlog "+- Draw: 18132.sda"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\graphics\so_bin\18132.sda"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\draw\level1\18132.sxd") , "StarOffice XML (Draw)") then
        call hCloseDocument
        sleep(2)
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\draw\level1\18132.sxd") , gOfficePath & ConvertPath("user\work\xml\draw\level1\18132\") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18132\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18132\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18132\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\draw\level1\18132.sxd")
            call hCloseDocument
        end if
    else
        call hCloseDocument
    end if
endcase
'
'-------------------------------------------------------------------------
'
testcase t18133_18134
    printlog "+- Draw: 18133-18134.sda"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\graphics\so_bin\18133-18134.sda"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\draw\level1\18133-18134.sxd") , "StarOffice XML (Draw)") then
        call hCloseDocument
        sleep(2)
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\draw\level1\18133-18134.sxd") , gOfficePath & ConvertPath("user\work\xml\draw\level1\18133-18134\") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18133-18134\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18133-18134\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18133-18134\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\draw\level1\18133-18134.sxd")
            call hCloseDocument
        end if
    else
        call hCloseDocument
    end if
endcase
'
'-------------------------------------------------------------------------
'
testcase t18135
    printlog "+- Draw: 18135.sda"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\graphics\so_bin\18135.sda"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\draw\level1\18135.sxd") , "StarOffice XML (Draw)") then
        call hCloseDocument
        sleep(2)
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\draw\level1\18135.sxd") , gOfficePath & ConvertPath("user\work\xml\draw\level1\18135\") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18135\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18135\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18135\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\draw\level1\18135.sxd")
            call hCloseDocument
        end if
    else
        call hCloseDocument
    end if
endcase
'
'-------------------------------------------------------------------------
'
testcase t18136
    printlog "+- Draw: 18136.sda"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\graphics\so_bin\18136.sda"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\draw\level1\18136.sxd") , "StarOffice XML (Draw)") then
        call hCloseDocument
        sleep(2)
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\draw\level1\18136.sxd") , gOfficePath & ConvertPath("user\work\xml\draw\level1\18136\") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18136\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18136\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18136\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\draw\level1\18136.sxd")
            call hCloseDocument
        end if
    else
        call hCloseDocument
    end if
endcase
'
'-------------------------------------------------------------------------
'
testcase t18137
    printlog "+- Draw: 18137.sda"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\graphics\so_bin\18137.sda"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\draw\level1\18137.sxd") , "StarOffice XML (Draw)") then
        call hCloseDocument
        sleep(2)
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\draw\level1\18137.sxd") , gOfficePath & ConvertPath("user\work\xml\draw\level1\18137\") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18137\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18137\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18137\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\draw\level1\18137.sxd")
            call hCloseDocument
        end if
    else
        call hCloseDocument
    end if
endcase
'
'-------------------------------------------------------------------------
'
testcase t18138
    printlog "+- Draw: 18138.sda"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\graphics\so_bin\18138.sda"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\draw\level1\18138.sxd") , "StarOffice XML (Draw)") then
        call hCloseDocument
        sleep(2)
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\draw\level1\18138.sxd") , gOfficePath & ConvertPath("user\work\xml\draw\level1\18138\") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18138\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18138\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18138\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\draw\level1\18138.sxd")
            call hCloseDocument
        end if
    else
        call hCloseDocument
    end if
endcase

