rem *************************************************************
rem  
rem  Licensed to the Apache Software Foundation (ASF) under one
rem  or more contributor license agreements.  See the NOTICE file
rem  distributed with this work for additional information
rem  regarding copyright ownership.  The ASF licenses this file
rem  to you under the Apache License, Version 2.0 (the
rem  "License"); you may not use this file except in compliance
rem  with the License.  You may obtain a copy of the License at
rem  
rem    http://www.apache.org/licenses/LICENSE-2.0
rem  
rem  Unless required by applicable law or agreed to in writing,
rem  software distributed under the License is distributed on an
rem  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
rem  KIND, either express or implied.  See the License for the
rem  specific language governing permissions and limitations
rem  under the License.
rem  
rem *************************************************************
'encoding UTF-8  Do not remove or change this line!
sub main
'  cMassentest
'  DisplayHid
  cTestdialog
  cSysDlgTest
  cFileOpenTest
  SidWintree

  FileExit
end sub

testcase cMassentest

DisplayHid
resetapplication
FileDialog
kontext "GrosserTestDlg"
dim c,t,lang,i
c = 0
lang = "0123456789abcdef"
lang = lang + lang
lang = lang + lang
lang = lang + lang
lang = lang + lang
lang = lang + lang
lang = lang + lang

lang = lang + lang
lang = lang + lang
nodebug
while 1
  c = c + 1
  t = str(c)
  MultiLineEdit.SetText t
  CheckBox.check lang
  CheckBox.uncheck lang
  for i = 1 to 200 : next
  beep
wend

endcase


testcase cFileOpenTest

  FileOpenTest
  setclipboard wintree
  kontext
  active.cancel

endcase


testcase cSysDlgTest

  SysDialogs
  setclipboard wintree
  kontext 
  active.yes
  setclipboard wintree
  active.ok
  active.Cancel

  SysDialogs
  active.Cancel
  active.ok

endcase

testcase cTestdialog

  FileDialog

  kontext "GrosserTestDlg"
  CheckBox.uncheck
  TriStateBox.tristate
  'OKButton
'  TimeField.settext("fhsdjk")
  MultiLineEdit.SetText "Das war der Text: '"+MultiLineEdit.GetText+"'"
  RadioButton1.check
  RadioButton2.check
'  MultiListBox.select 2
  ComboBox.select("Dritter")
  DateBox.select("1.1.91")

  GrosserTestDlg.ok

endcase


sub LoadIncludeFiles

  start "miniapp.exe", "-enableautomation"

  use "test.win"
  use "test.sid"

  testexit

end sub

sub testenter
end sub

sub testexit

  dim xx
  xx = resetapplication
  if xx > "" then warnlog xx

end sub














