sub main

wintree
'  cMassentest
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
'lang = lang + lang
'lang = lang + lang
'lang = lang + lang
'print len(lang)
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
  wintree
  kontext
  active.cancel

endcase


testcase cSysDlgTest

  SysDialogs
  wintree
  kontext 
  active.yes
  wintree
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
'  start "d:\prj\actual\basic\source\testtool\server\debug\server.exe"
  start "d:\office40.vcl\miniapp.exe"


  use "test.win"
  use "test.sid"

  testexit

end sub

sub testenter
end sub

sub testexit
'  if GrosserTestDlg.exists (1) then
'    GrosserTestDlg.ok
'  endif

  dim xx
  xx = GetNextError
  if xx > "" then print xx
  xx = resetapplication
  if xx > "" then warnlog xx

end sub














