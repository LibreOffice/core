'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Function doUnitTest
  ''' &h &o literals - Integers only for the moment '''  
  If Not test_h_literals Or Not test_o_literals Then
    doUnitTest = 0
  Else
    doUnitTest = 1 'Ok
  End If
End Function

Sub DEV_TEST : MsgBox doUnitTest : End Sub

Function test_h_literals As Boolean
  ' INTEGER // MIN=-2^15 // MAX=2^15-1
  isTrimmed = (&h0007FFF = +32767 And &h00008000 = -32768)
  isInteger_OK = isTrimmed And Typename(&h008000)="Integer" ' represents -1

  ' LONG // MIN=-2^31 // MAX=2^31-1
  ' isTrimmedToo = (&h007FFFFFFF = 2^31-1 And &h000080000000 = 2^31)
  ' isLong_OK = isTrimmedToo ' And Typename(&h00fedcba98)="Long")
  ' Typename returns Double instead of Long

  test_h_literals = isInteger_OK 'And isLong_OK
End Function

Function test_o_literals As Boolean
  ' INTEGER // MIN=-2^15 // MAX=2^15-1
  isTrimmed = (&o00077777 = +32767 And &o0000100000 = -32768)
  isInteger_OK = isTrimmed And Typename(&o00177777)="Integer" ' represents -1

  'isLong_OK = isTrimmedToo And Typename(..)="Long" TO BE DONE
  
  test_o_literals = isInteger_OK 'And isLong_OK
End Function