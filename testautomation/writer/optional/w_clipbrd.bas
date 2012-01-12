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
