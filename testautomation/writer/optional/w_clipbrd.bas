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
'* short description : CROSS-APPLICATIONS CLIPBOARD TEST (Text only)
'*
'\***********************************************************************

Global CharsetSet as integer
Global ExportSet as integer

sub main
    use "writer\tools\includes\w_tools.inc"
    use "writer\optional\includes\clipboard\clipbrd_func.inc"
    use "writer\optional\includes\clipboard\w_210_.inc"
    use "writer\optional\includes\clipboard\w_211_.inc"
    use "writer\optional\includes\clipboard\w_212_.inc"
    use "writer\optional\includes\clipboard\w_213_.inc"
    use "writer\optional\includes\clipboard\w_214_.inc"
    use "writer\optional\includes\clipboard\w_215_.inc"
    use "writer\optional\includes\clipboard\w_216_.inc"
    use "writer\optional\includes\clipboard\w_217_.inc"
    use "writer\optional\includes\clipboard\w_218_.inc"
    use "writer\optional\includes\clipboard\w_219_.inc"
    use "writer\optional\includes\clipboard\w_220_.inc"
    use "writer\optional\includes\clipboard\w_221_.inc"
    use "writer\optional\includes\clipboard\w_222_.inc"
    Dim Start : Start = Now()
    Dim lsSourceList(3) as string

    ' copy needed files linked in test documents lo local file system if needed Issue #112208
    lsSourceList(0) = 3
    lsSourceList(1) = ConvertPath ( gtesttoolpath & "writer\optional\input\clipboard\dummy.sxw" )
    lsSourceList(2) = ConvertPath ( gtesttoolpath & "writer\optional\input\clipboard\dummy.txt" )
    lsSourceList(3) = ConvertPath ( gtesttoolpath & "writer\optional\input\clipboard\flower.gif" )

    if hFileListCopyLocal(lsSourceList()) = false then
        warnlog "Copying of elementary files failed. Test interrupted!"
        exit sub
    end if

    ExportSet = wChangeHTMLCompatibilityExport
    CharsetSet = wChangeHTMLCompatibility
    Call hStatusIn("writer","w_clipbrd.bas","CROSS-APPLICATIONS CLIPBOARD TEST (Text only)")

    Call w_210_
    Call w_211_
    Call w_212_
    Call w_213_
    Call w_214_
    Call w_215_
    Call w_216_
    Call w_217_
    Call w_218_
    Call w_219_
    Call w_220_
    Call w_221_
    Call w_222_

    Call hStatusOut
    Call wChangeHTMLCompatibility(CharsetSet)
    Call wChangeHTMLCompatibilityExport(ExportSet)

    printlog "Duration : " +  Wielange ( Start )
end sub

'---------------------------------------------------------
sub LoadIncludeFiles
   use "global\system\includes\master.inc"
   use "global\system\includes\gvariabl.inc"
   gApplication = "WRITER"
   Call GetUseFiles
end sub
