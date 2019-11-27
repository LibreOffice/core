'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Function doUnitTest
  Dim s1 As Integer, s2 As Integer, value As Integer

  ' FOR EACH
  numbers = Array(3,4,5,6,7,8,9,10,11)
  For Each value in numbers
    s1 = s1 + value
  Next value

  ' FOR .. TO .. STEP ..
  For value = 5 To 21 Step 3
    s2 = s2 + value
  Next value

  If s1<>9*7 Or s2<>3*25 Then
    doUnitTest = 0
  Else
    doUnitTest = 1 'Ok
  End If
End Function