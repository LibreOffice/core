'
' This file is part of the LibreOffice project.
'
' This Source Code Form is subject to the terms of the Mozilla Public
' License, v. 2.0. If a copy of the MPL was not distributed with this
' file, You can obtain one at http://mozilla.org/MPL/2.0/.
'

Option Explicit

Function doUnitTest() As String
    TestUtil.TestInit
    verify_Collection
    doUnitTest = TestUtil.GetResult()
End Function

Sub DEV_TEST : MsgBox doUnitTest() : End Sub

Sub verify_Collection()

try: On Error GoTo catch

    Dim c As New Collection, planet as String, ndx As Integer

    TestUtil.assertEqual(c.Count, 0,"c.Count")

    ' Solar system planets from closest to SUN
    ' ranked alphabetically using before/after keywords
    c.Add("Mercury")
    c.Add("Venus", "♀")
    c.Add("Earth", "♁", before:=1)
    c.Add("Mars", "♂", after:="♁")
    c.Add("Jupiter", after:="♁")
    c.Add("Saturn", before:=5)

    TestUtil.assertEqual(c.Count, 6,"c.Count")
    TestUtil.assertEqual(c.Item(6), "Venus","c.Item(6)")
    TestUtil.assertEqual(c.Item("♂"), "Mars","c.Item(""♂"")")

    c.Add("Uranus", before:="♀")
    c.Add("Neptune", "♆", after:=4)
    'For Each planet in c : Print planet, : Next

    TestUtil.assertEqual(c.Count, 8,"c.Count")

    c.remove(4)
    c.remove("♁")

    TestUtil.assertEqual(c.Count, 7,"c.Count")

    For ndx = c.Count to 1 Step -1
        c.Remove(ndx)
    Next ndx

    TestUtil.assertEqual(c.Count, 0,"c.Count")

finally:
    Exit Sub

catch:
    TestUtil.ReportErrorHandler("verify_Collection", Err, Error$, Erl)

End Sub
