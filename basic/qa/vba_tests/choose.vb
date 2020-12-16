Option VBASupport 1
Option Explicit

Function doUnitTest() As String
verify_testChoose
doUnitTest = TestUtilModule.GetResult()
End Function

Sub verify_testChoose()

    TestUtilModule.TestInit

    Dim testName As String
    Dim var1, var2
    testName = "Test Choose function"

    On Error GoTo errorHandler

    var2 = "Libre"
    var1 = Choose(1, "Libre", "Office", "Suite")
    TestUtilModule.AssertTrue(var1 = var2, "the return Choose is: " & var1)

    var2 = "Office"
    var1 = Choose(2, "Libre", "Office", "Suite")
    TestUtilModule.AssertTrue(var1 = var2, "the return Choose is: " & var1)

    var2 = "Suite"
    var1 = Choose(3, "Libre", "Office", "Suite")
    TestUtilModule.AssertTrue(var1 = var2, "the return Choose is: " & var1)

    var1 = Choose(4, "Libre", "Office", "Suite")
    TestUtilModule.AssertTrue(IsNull(var1), "the return Choose is: Null4 "  )

    var1 = Choose(0, "Libre", "Office", "Suite")
    TestUtilModule.AssertTrue(IsNull(var1), "the return Choose is: Null0 " )

    var1 = Choose(-1, "Libre", "Office", "Suite")
    TestUtilModule.AssertTrue(IsNull(var1), "the return Choose is: Null-1")

    TestUtilModule.TestEnd

    Exit Sub
errorHandler:
        TestUtilModule.AssertTrue(False, testName & ": hit error handler")
End Sub

