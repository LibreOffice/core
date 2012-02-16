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
'* short description : Writer Filter Availability Test (Import/Export/Group names)
'*
'\*******************************************************************

global gSeperator as String
global gMeasurementUnit as String

sub main
 Dim ImportFilterDatei as String

  use "writer\tools\includes\w_tool6.inc"
  use "writer\optional\includes\filter\w_filter.inc"

   Call hStatusIn ( "writer", "w_filter.bas" )

   Call tSammleImportFilter
   Call tCollectSaveAsFilter
   Call tCollectExportFilters
   Call tGroupNameCheck

   Call hStatusOut

end sub

sub LoadIncludeFiles
   use "global\system\includes\master.inc"
   use "global\system\includes\gvariabl.inc"
   use "global\tools\includes\optional\t_xml1.inc"
   gApplication   = "WRITER"
   Call GetUseFiles()
end sub
