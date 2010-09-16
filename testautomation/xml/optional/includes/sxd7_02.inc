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

sub sxd7_02

   printlog "------------------- sxd_02.inc ---------------------"

   Call ExitRestartTheOffice
   call t18108_18115
   call t18116
   call t18117
   call t18118
   call t18119
   call t18120
   call t18121_18122
   call t18123   
   call t18124
   call t18125

end sub
'
'-------------------------------------------------------------------------
'
testcase t18108_18115
    printlog "+- Draw: 18108-18115.sda"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\graphics\so_bin\18108-18115.sda"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\draw\level1\18108-18115.sxd") , "StarOffice XML (Draw)") then
        call hCloseDocument
        sleep(2)
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\draw\level1\18108-18115.sxd") , gOfficePath & ConvertPath("user\work\xml\draw\level1\18108-18115\") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18108-18115\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18108-18115\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18108-18115\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\draw\level1\18108-18115.sxd")
            call hCloseDocument
        end if
    else
        call hCloseDocument
    end if
endcase
'
'-------------------------------------------------------------------------
'
testcase t18116
    printlog "+- Draw: 18116.sda"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\graphics\so_bin\18116.sda"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\draw\level1\18116.sxd") , "StarOffice XML (Draw)") then
        call hCloseDocument
        sleep(2)
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\draw\level1\18116.sxd") , gOfficePath & ConvertPath("user\work\xml\draw\level1\18116\") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18116\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18116\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18116\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\draw\level1\18116.sxd")
            call hCloseDocument
        end if
    else
        call hCloseDocument
    end if
endcase
'
'-------------------------------------------------------------------------
'
testcase t18117
    printlog "+- Draw: 18117.sda"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\graphics\so_bin\18117.sda"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\draw\level1\18117.sxd") , "StarOffice XML (Draw)") then
        call hCloseDocument
        sleep(2)
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\draw\level1\18117.sxd") , gOfficePath & ConvertPath("user\work\xml\draw\level1\18117\") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18117\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18117\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18117\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\draw\level1\18117.sxd")
            call hCloseDocument
        end if
    else
        call hCloseDocument
    end if
endcase
'
'-------------------------------------------------------------------------
'
testcase t18118
    printlog "+- Draw: 18118.sda"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\graphics\so_bin\18118.sda"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\draw\level1\18118.sxd") , "StarOffice XML (Draw)") then
        call hCloseDocument
        sleep(2)
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\draw\level1\18118.sxd") , gOfficePath & ConvertPath("user\work\xml\draw\level1\18118\") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18118\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18118\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18118\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\draw\level1\18118.sxd")
            call hCloseDocument
        end if
    else
        call hCloseDocument
    end if
endcase
'
'-------------------------------------------------------------------------
'
testcase t18119
    printlog "+- Draw: 18119.sda"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\graphics\so_bin\18119.sda"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\draw\level1\18119.sxd") , "StarOffice XML (Draw)") then
        call hCloseDocument
        sleep(2)
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\draw\level1\18119.sxd") , gOfficePath & ConvertPath("user\work\xml\draw\level1\18119\") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18119\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18119\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18119\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\draw\level1\18119.sxd")
            call hCloseDocument
        end if
    else
        call hCloseDocument
    end if
endcase
'
'-------------------------------------------------------------------------
'
testcase t18120
    printlog "+- Draw: 18120.sda"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\graphics\so_bin\18120.sda"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\draw\level1\18120.sxd") , "StarOffice XML (Draw)") then
        call hCloseDocument
        sleep(2)
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\draw\level1\18120.sxd") , gOfficePath & ConvertPath("user\work\xml\draw\level1\18120\") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18120\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18120\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18120\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\draw\level1\18120.sxd")
            call hCloseDocument
        end if
    else
        call hCloseDocument
    end if
endcase
'
'-------------------------------------------------------------------------
'
testcase t18121_18122
    printlog "+- Draw: 18121-18122.sda"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\graphics\so_bin\18121-18122.sda"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\draw\level1\18121-18122.sxd") , "StarOffice XML (Draw)") then
        call hCloseDocument
        sleep(2)
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\draw\level1\18121-18122.sxd") , gOfficePath & ConvertPath("user\work\xml\draw\level1\18121-18122\") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18121-18122\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18121-18122\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18121-18122\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\draw\level1\18121-18122.sxd")
            call hCloseDocument
        end if
    else
        call hCloseDocument
    end if
endcase
'
'-------------------------------------------------------------------------
'
testcase t18123
    printlog "+- Draw: 18123.sda"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\graphics\so_bin\18123.sda"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\draw\level1\18123.sxd") , "StarOffice XML (Draw)") then
        call hCloseDocument
        sleep(2)
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\draw\level1\18123.sxd") , gOfficePath & ConvertPath("user\work\xml\draw\level1\18123\") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18123\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18123\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18123\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\draw\level1\18123.sxd")
            call hCloseDocument
        end if
    else
        call hCloseDocument
    end if
endcase
'
'-------------------------------------------------------------------------
'
testcase t18124
    printlog "+- Draw: 18124.sda"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\graphics\so_bin\18124.sda"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\draw\level1\18124.sxd") , "StarOffice XML (Draw)") then
        call hCloseDocument
        sleep(2)
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\draw\level1\18124.sxd") , gOfficePath & ConvertPath("user\work\xml\draw\level1\18124\") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18124\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18124\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18124\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\draw\level1\18124.sxd")
            call hCloseDocument
        end if
    else
        call hCloseDocument
    end if
endcase
'
'-------------------------------------------------------------------------
'
testcase t18125
    printlog "+- Draw: 18125.sda"
    call hFileOpen (gTesttoolPath & ConvertPath("xml\optional\input\graphics\so_bin\18125.sda"))
    if hFileSaveAsWithFilterKill (gOfficePath & ConvertPath("user\work\xml\draw\level1\18125.sxd") , "StarOffice XML (Draw)") then
        call hCloseDocument
        sleep(2)
        UnpackStorage( gOfficePath & ConvertPath("user\work\xml\draw\level1\18125.sxd") , gOfficePath & ConvertPath("user\work\xml\draw\level1\18125\") )
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18125\styles.xml")) = FALSE then
            warnlog "XML-file 'styles.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18125\meta.xml")) = FALSE then
            warnlog "XML-file 'meta.xml' not well formed!"
        end if
        if XMLWellformed (gOfficePath & ConvertPath("user\work\xml\draw\level1\18125\content.xml")) = FALSE then
            warnlog "XML-file 'content.xml' not well formed!"
        else
            call hFileOpen (gOfficePath & ConvertPath("user\work\xml\draw\level1\18125.sxd")
            call hCloseDocument
        end if
    else
        call hCloseDocument
    end if
endcase

