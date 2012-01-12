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
' **
' ** short description : check the internal file dialog (Dialog)
' **
'\******************************************************************************

sub main

    use "framework\optional\includes\filedlg_triggers.inc"
    use "framework\optional\includes\filedlg_rename.inc"
    use "framework\optional\includes\filedlg_document_properties.inc"
    use "framework\optional\includes\filedlg_filternames.inc"

    call hStatusIn ( "framework", "f_filedlg_dialogtest.bas" )
    call tDialogTriggers()         
    call tFiledlgDocumentProperties()
    call tVerifyFilterNames()
    call tFileRename()
    call hStatusOut

end sub

' ****************************************

sub LoadIncludeFiles

    use "global\system\includes\master.inc"
    use "global\system\includes\gvariabl.inc"
   
    use "global\tools\includes\optional\t_stringtools.inc"
    use "global\tools\includes\optional\t_listfuncs.inc"    
    use "global\tools\includes\optional\t_filetools.inc"
    use "global\tools\includes\optional\t_docfuncs.inc"
    use "global\tools\includes\optional\t_accels.inc"

    gApplication = "WRITER"
    Call GetUseFiles

end sub
