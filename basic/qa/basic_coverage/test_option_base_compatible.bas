Option Base 1
Option Compatible
Option Explicit

Function doUnitTest() As String
    TestUtil.TestInit
    verify_optionBase
    doUnitTest = TestUtil.GetResult()
End Function

Sub verify_optionBase()

    On Error GoTo errorHandler

    ' tdf#54912 - with option base arrays should start at index 1.
    ' With option compatible the upper bound is not changed (#109275).
    Dim strArray(2) As String
    TestUtil.AssertEqual(LBound(strArray), 1, "Lower bound of a string array (before assignment): " & LBound(strArray))
    TestUtil.AssertEqual(UBound(strArray), 2, "Upper bound of a string array (before assignment): " & UBound(strArray))
    strArray = Array("a", "b")
    TestUtil.AssertEqual(LBound(strArray), 1, "Lower bound of a string array (after assignment): " & LBound(strArray))
    TestUtil.AssertEqual(UBound(strArray), 2, "Upper bound of a string array (after assignment): " & UBound(strArray))

    Dim intArray(2) As Integer
    TestUtil.AssertEqual(LBound(intArray), 1, "Lower bound of an integer array (before assignment): " & LBound(intArray))
    TestUtil.AssertEqual(UBound(intArray), 2, "Upper bound of an integer array (before assignment): " & UBound(intArray))
    intArray = Array(1, 2)
    TestUtil.AssertEqual(LBound(intArray), 1, "Lower bound of an integer array (after assignment): " & LBound(intArray))
    TestUtil.AssertEqual(UBound(intArray), 2, "Upper bound of an integer array (after assignment): " & UBound(intArray))

    Dim byteArray(2) As Byte
    TestUtil.AssertEqual(LBound(byteArray), 1, "Lower bound of a byte array (before assignment): " & LBound(byteArray))
    TestUtil.AssertEqual(UBound(byteArray), 2, "Upper bound of a byte array (before assignment): " & UBound(byteArray))
    byteArray = StrConv("ab", 128)
    TestUtil.AssertEqual(LBound(byteArray), 1, "Lower bound of a byte array (StrConv): " & LBound(byteArray))
    TestUtil.AssertEqual(UBound(byteArray), 2, "Upper bound of a byte array (StrConv): " & UBound(byteArray))

    ReDim byteArray(3)
    TestUtil.AssertEqual(LBound(byteArray), 1, "Lower bound of a byte array (ReDim): " & LBound(byteArray))
    TestUtil.AssertEqual(UBound(byteArray), 3, "Upper bound of a byte array (ReDim): " & UBound(byteArray))

    Exit Sub
errorHandler:
    TestUtil.ReportErrorHandler("verify_optionBase", Err, Error$, Erl)
End Sub
