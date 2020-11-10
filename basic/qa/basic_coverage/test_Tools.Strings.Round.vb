'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Option Compatible

Function doUnitTest as Integer
  ' Tools.Misc.Round() testcase
  GlobalScope.BasicLibraries.LoadLibrary("Tools")
  baseValue1 = 1.2056E-2 : decimals = 3
  baseValue2 = -4.6822E+1
  result1 = Tools.Strings.Round(baseValue1, decimals)
  result2 = Round(baseValue2, 2)
  If ( result1 <> Cdbl(1.2E-2) Or result2 <> Cdbl(-46.56) ) Then
    doUnitTest = 0 ' Ko
  Else
    doUnitTest = 1 ' Ok
  End If
End Function

Sub DEV_TEST : MsgBox doUnitTest : End Sub  
