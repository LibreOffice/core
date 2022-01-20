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
    Call verify_Collection
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_Collection()
try: On Error GoTo catch

    Dim c As New Collection, planet as String, ndx As Integer

	TestUtil.assertEqual(c.Count, 0,"c.Count")

	' Let's collect Solar system planets from closest to SUN,
	' ranking them alphabetically using before/after keywords,
	' in order to validate various Add() method syntax variations.
	c.Add("Mercury")
	c.Add("Venus", "♀")
	c.Add("Earth", "♁", before:=1)
	TestUtil.assertEqual(c.Count, 3,"c.Count")
	TestUtil.assertEqual(c.Item(1), "Earth","c.Item(1)")
	TestUtil.assertEqual(c.Item("♁"), "Earth","c.Item(""♁"")")
	TestUtil.assertEqual(c.Item(3), "Venus","c.Item(3)")
	TestUtil.assertEqual(c.Item("♀"), "Venus","c.Item(""♀"")")

	c.Add("Mars", "♂", after:="♁")
	c.Add("Jupiter", after:="♁")
	c.Add("Saturn", before:=5)
	TestUtil.assertEqual(c.Count, 6,"c.Count")
	TestUtil.assertEqual(c.Item(2), "Jupiter","c.Item(2)")
	TestUtil.assertEqual(c.Item(3), "Mars","c.Item(3)")
	TestUtil.assertEqual(c.Item("♂"), "Mars","c.Item(""♂"")")
	TestUtil.assertEqual(c.Item(5), "Saturn","c.Item(5)")
	TestUtil.assertEqual(c.Item(6), "Venus","c.Item(6)")

	c.Add("Uranus", before:="♀")
	c.Add("Neptune", "♆", after:=4)
	TestUtil.assertEqual(c.Count, 8,"c.Count")
	TestUtil.assertEqual(c.Item(7), "Uranus","c.Item(7)")
	TestUtil.assertEqual(c.Item(5), "Neptune","c.Item(5)")
	TestUtil.assertEqual(c.Item("♆"), "Neptune","c.Item(""♆"")")
	TestUtil.assertEqual(c.Item(6), "Saturn","c.Item(6)")

	c.remove(4)
	c.remove("♁")

	TestUtil.assertEqual(c.Count, 6,"c.Count")

	For ndx = c.Count to 1 Step -1
		c.Remove(ndx)
	Next ndx

	TestUtil.assertEqual(c.Count, 0,"c.Count")

finally:
    Exit Sub

catch:
    TestUtil.ReportErrorHandler("verify_Collection", Err, Error$, Erl)
    Resume Next
End Sub
