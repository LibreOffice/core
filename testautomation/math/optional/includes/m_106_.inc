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
'* short description :
'*
'\******************************************************************

sub M_106_
   printlog Chr(13) +  "--  Tools-Menue  --"
   Call tExtrasFormelImportieren
end sub

testcase tExtrasFormelImportieren
 dim i,x as integer

'/// check if formula is loaded in same window ///'

   Call hNewDocument

  '/// check amount of available filters (6) (inc. one seperator) ///'
   ToolsImportFormula
      Kontext "OeffnenDlg"
      x = Dateityp.GetItemCount
      if x <> cFilterCountLoad then warnlog " The Filter count is wrong! Have to be ("+cFilterCountLoad+"), but are: "+x
      printlog "------------- this is the filter list: ------------"
      for i = 1 to x
         try
            Printlog " "+i+": " +  Dateityp.GetItemText (i)
          catch
            printlog "fail ------------"
          endcatch
      next i
   OeffnenDlg.Cancel
   Sleep 2

   Call hCloseDocument
endcase


