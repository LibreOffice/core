Option VBASupport 1
Option Explicit

Dim passCount As Integer
Dim failCount As Integer
Dim result As String

Function doUnitTest() As String
result = verify_format()
If failCount <> 0 And passCount > 0 Then
    doUnitTest = result
Else
    doUnitTest = "OK"
End If
End Function

Function verify_format() as String
    passCount = 0
    failCount = 0

    result = "Test Results" & Chr$(10) & "============" & Chr$(10)
    
    'Predefined_Datetime_Format_Sample
    Predefined_Number_Format_Sample
    'Custom_Datetime_Format_Sample
    Custom_Number_Format_Sample
    Custom_Text_Format_Sample
    result = result & Chr$(10) & "Tests passed: " & passCount & Chr$(10) & "Tests failed: " & failCount & Chr$(10)
    verify_format = result
    
End Sub


Sub Predefined_Datetime_Format_Sample()
    Dim testName As String
    Dim myDate, MyTime, TestStr As String
    myDate = "01/06/98"
    MyTime = "17:08:06"
    testName = "Test Predefined_Datetime_Format_Sample function"
    
    On Error GoTo errorHandler
    
    ' The date/time format have a little different between ms office and OOo due to different locale and system...
    TestStr = Format(myDate, "General Date") ' 1/6/98
    
    TestLog_ASSERT IsDate(TestStr), "General Date: " & TestStr & " (Test only applies to en_US locale)"
    'TestLog_ASSERT TestStr = "1/6/98", "General Date: " & TestStr
    
    TestStr = Format(myDate, "Long Date") ' Tuesday, January 06, 1998
    TestLog_ASSERT TestStr = "Tuesday, January 06, 1998", "Long Date: " & TestStr & " (Test only applies to en_US locale)"
    'TestLog_ASSERT IsDate(TestStr), "Long Date: " & TestStr
    
    TestStr = Format(myDate, "Medium Date") ' 06-Jan-98
    'TestLog_ASSERT TestStr = "06-Jan-98", "Medium Date: " & TestStr
    TestLog_ASSERT IsDate(TestStr), "Medium Date: " & TestStr & " (Test only applies to en_US locale)"
    
    
    TestStr = Format(myDate, "Short Date") ' 1/6/98
    'TestLog_ASSERT TestStr = "1/6/98", "Short Date: " & TestStr
    TestLog_ASSERT IsDate(TestStr), "Short Date: " & TestStr & " (Test only applies to en_US locale)"
    
    TestStr = Format(MyTime, "Long Time") ' 5:08:06 PM
    'TestLog_ASSERT TestStr = "5:08:06 PM", "Long Time: " & TestStr
    TestLog_ASSERT IsDate(TestStr), "Long Time: " & TestStr & " (Test only applies to en_US locale)"
    
    
    TestStr = Format(MyTime, "Medium Time") ' 05:08 PM
    'TestLog_ASSERT TestStr = "05:08 PM", "Medium Time: " & TestStr
    TestLog_ASSERT IsDate(TestStr), "Medium Time: " & TestStr & " (Test only applies to en_US locale)"
    
    
    TestStr = Format(MyTime, "Short Time") ' 17:08
    'TestLog_ASSERT TestStr = "17:08", "Short Time: " & TestStr
    TestLog_ASSERT IsDate(TestStr), "Short Time: " & TestStr & " (Test only applies to en_US locale)"
    Exit Sub
errorHandler:
        TestLog_ASSERT (false), testName & ": hit error handler"
End Sub

Sub Predefined_Number_Format_Sample()
    Dim myNumber, TestStr As String
    Dim testName As String
    testName = "Test Predefined_Number_Format_Sample function"
    myNumber = 562486.2356
    
     On Error GoTo errorHandler
    
    TestStr = Format(myNumber, "General Number") '562486.2356
    TestLog_ASSERT TestStr = "562486.2356", "General Number: " & TestStr
    'MsgBox TestStr
    
    TestStr = Format(0.2, "Fixed") '0.20
    TestLog_ASSERT TestStr = "0.20", "Fixed: " & TestStr
    'MsgBox TestStr
    
    TestStr = Format(myNumber, "Standard") '562,486.24
    TestLog_ASSERT TestStr = "562,486.24", "Standard: " & TestStr
    'MsgBox TestStr
    
    TestStr = Format(0.7521, "Percent") '75.21%
    TestLog_ASSERT TestStr = "75.21%", "Percent: " & TestStr
    'MsgBox TestStr
    
    TestStr = Format(myNumber, "Scientific") '5.62E+05
    TestLog_ASSERT TestStr = "5.62E+05", "Scientific: " & TestStr
    'MsgBox TestStr
    
    TestStr = Format(-3456.789, "Scientific") '-3.46E+03
    TestLog_ASSERT TestStr = "-3.46E+03", "Scientific: " & TestStr
    'MsgBox TestStr
    
    TestStr = Format(0, "Yes/No") 'No
    TestLog_ASSERT TestStr = "No", "Yes/No: " & TestStr
    'MsgBox TestStr
    
    TestStr = Format(23, "Yes/No") 'Yes
    TestLog_ASSERT TestStr = "Yes", "Yes/No: " & TestStr
    'MsgBox TestStr
    
    TestStr = Format(0, "True/False") 'False
    TestLog_ASSERT TestStr = "False", "True/False: " & TestStr
    'MsgBox TestStr
    
    TestStr = Format(23, "True/False") 'True
    TestLog_ASSERT TestStr = "True", "True/False: " & TestStr
    'MsgBox TestStr
    
    TestStr = Format(0, "On/Off") 'Off
    TestLog_ASSERT TestStr = "Off", "On/Off: " & TestStr
    'MsgBox TestStr
    
    TestStr = Format(23, "On/Off") 'On
    TestLog_ASSERT TestStr = "On", "On/Off: " & TestStr
    'MsgBox TestStr
    
    Exit Sub
errorHandler:
        TestLog_ASSERT (false), testName & ": hit error handler"
    
End Sub

Sub Custom_Datetime_Format_Sample()
    Dim myDate, MyTime, TestStr As String
    Dim testName As String
    
    myDate = "01/06/98"
    MyTime = "05:08:06"
    
    testName = "Test Custom_Datetime_Format_Sample function"
     On Error GoTo errorHandler
    
    TestStr = Format("01/06/98 17:08:06", "c") ' 1/6/98 5:08:06 PM
    TestLog_ASSERT TestStr = "1/6/98 5:08:06 PM", "c: " & TestStr & " (Test only applies to en_US locale)"
    'MsgBox TestStr
    
    TestStr = Format(myDate, "dddddd") ' (Long Date), Tuesday, January 06, 1998
    TestLog_ASSERT TestStr = "Tuesday, January 06, 1998", "dddddd: " & TestStr & " (Test only applies to en_US locale)"
    'MsgBox TestStr
    
    TestStr = Format(myDate, "mm-dd-yyyy") ' 01-06-19s98
    TestLog_ASSERT TestStr = "01-06-1998", "mm-dd-yyyy: " & TestStr & " (Test only applies to en_US locale)"
    'MsgBox TestStr
    
    TestStr = Format(myDate, "d") ' 6
    TestLog_ASSERT TestStr = "6", "d: " & TestStr & " (Test only applies to en_US locale)"
    'MsgBox TestStr
    
    TestStr = Format(myDate, "dd") ' 06
    TestLog_ASSERT TestStr = "06", "dd: " & TestStr & " (Test only applies to en_US locale)"
    'MsgBox TestStr
    
    TestStr = Format(myDate, "ddd") ' Tue
    TestLog_ASSERT TestStr = "Tue", "ddd: " & TestStr & " (Test only applies to en_US locale)"
    'MsgBox TestStr
    
    TestStr = Format(myDate, "dddd") ' Tuesday
    TestLog_ASSERT TestStr = "Tuesday", "dddd: " & TestStr & " (Test only applies to en_US locale)"
    'MsgBox TestStr
       
    TestStr = Format(MyTime, "h") ' 5
    TestLog_ASSERT TestStr = "5", "h: " & TestStr & " (Test only applies to en_US locale)"
    'MsgBox TestStr
    
    TestStr = Format(MyTime, "hh") ' 05
    TestLog_ASSERT TestStr = "05", "hh: " & TestStr & " (Test only applies to en_US locale)"
    'MsgBox TestStr
    
    TestStr = Format(MyTime, "n") ' 8
    TestLog_ASSERT TestStr = "8", "n: " & TestStr & " (Test only applies to en_US locale)"
    'MsgBox TestStr
    
    TestStr = Format(MyTime, "nn") ' 08
    TestLog_ASSERT TestStr = "08", "nn: " & TestStr & " (Test only applies to en_US locale)"
    'MsgBox TestStr
    
    TestStr = Format(myDate, "m") ' 1
    TestLog_ASSERT TestStr = "1", "m: " & TestStr & " (Test only applies to en_US locale)"
    'MsgBox TestStr
    
    TestStr = Format(myDate, "mm") ' 01
    TestLog_ASSERT TestStr = "01", "mm: " & TestStr & " (Test only applies to en_US locale)"
    'MsgBox TestStr
    
    TestStr = Format(myDate, "mmm") ' Jan
    TestLog_ASSERT TestStr = "Jan", "mmm: " & TestStr & " (Test only applies to en_US locale)"
    'MsgBox TestStr
    
    TestStr = Format(myDate, "mmmm") ' January
    TestLog_ASSERT TestStr = "January", "mmmm: " & TestStr & " (Test only applies to en_US locale)"
    'MsgBox TestStr
    
    TestStr = Format(MyTime, "s") ' 6
    TestLog_ASSERT TestStr = "6", "s: " & TestStr & " (Test only applies to en_US locale)"
    'MsgBox TestStr
    
    TestStr = Format(MyTime, "ss") ' 06
    TestLog_ASSERT TestStr = "06", "ss: " & TestStr & " (Test only applies to en_US locale)"
    'MsgBox TestStr
   
    
    MyTime = "17:08:06"
    
    TestStr = Format(MyTime, "hh:mm:ss AM/PM") ' 05:08:06 PM
    TestLog_ASSERT TestStr = "05:08:06 PM", "hh:mm:ss AM/PM: " & TestStr & " (Test only applies to en_US locale)"
  
    
    TestStr = Format(MyTime, "hh:mm:ss") ' 17:08:06
    TestLog_ASSERT TestStr = "17:08:06", "hh:mm:ss: " & TestStr & " (Test only applies to en_US locale)"
    'MsgBox TestStr
    
    TestStr = Format(myDate, "ww") ' 2
    TestLog_ASSERT TestStr = "2", "ww: " & TestStr & " (Test only applies to en_US locale)"
    'MsgBox TestStr
    
    TestStr = Format(myDate, "w") ' 3
    TestLog_ASSERT TestStr = "3", "w: " & TestStr & " (Test only applies to en_US locale)"
    'MsgBox TestStr
    
    TestStr = Format(myDate, "y") ' 6
    TestLog_ASSERT TestStr = "6", "y: " & TestStr & " (Test only applies to en_US locale)"
    'MsgBox TestStr
    
    TestStr = Format(myDate, "yy") ' 98
    TestLog_ASSERT TestStr = "98", "yy: " & TestStr & " (Test only applies to en_US locale)"
    'MsgBox TestStr
    
    TestStr = Format(myDate, "yyyy") ' 1998
    TestLog_ASSERT TestStr = "1998", "yyyy: " & TestStr & " (Test only applies to en_US locale)"
    'MsgBox TestStr
    
    Exit Sub
errorHandler:
        TestLog_ASSERT (false), testName & ": hit error handler"
End Sub

Sub Custom_Number_Format_Sample()
    Dim TestStr As String
    Dim testName As String
    
    testName = "Test Custom_Number_Format_Sample function"
     On Error GoTo errorHandler
    
    TestStr = Format(23.675, "00.0000") ' 23.6750
    TestLog_ASSERT TestStr = "23.6750", "00.0000: " & TestStr
    'MsgBox TestStr
    
    TestStr = Format(23.675, "00.00") ' 23.68
    TestLog_ASSERT TestStr = "23.68", "00.00: " & TestStr
    'MsgBox TestStr
    
    TestStr = Format(2658, "00000") ' 02658
    TestLog_ASSERT TestStr = "02658", "00000: " & TestStr
    'MsgBox TestStr
    
    TestStr = Format(2658, "00.00") ' 2658.00
    TestLog_ASSERT TestStr = "2658.00", "00.00: " & TestStr
    'MsgBox TestStr
    
    TestStr = Format(23.675, "##.####") ' 23.675
    TestLog_ASSERT TestStr = "23.675", "##.####: " & TestStr
    'MsgBox TestStr
    
    TestStr = Format(23.675, "##.##") ' 23.68
    TestLog_ASSERT TestStr = "23.68", "##.##: " & TestStr
    'MsgBox TestStr
    
    TestStr = Format(12345.25, "#,###.##") '12,345.25
    TestLog_ASSERT TestStr = "12,345.25", "#,###.##: " & TestStr
    'MsgBox TestStr
    
    TestStr = Format(0.25, "##.00%") '25.00%
    TestLog_ASSERT TestStr = "25.00%", "##.00%: " & TestStr
    'MsgBox TestStr
    
    TestStr = Format(1000000, "#,###") '1,000,000
    TestLog_ASSERT TestStr = "1,000,000", "#,###: " & TestStr
    'MsgBox TestStr
    
    TestStr = Format(1.09837555, "######E-###") '109838E-5
    TestLog_ASSERT TestStr = "109838E-5", "######E-###: " & TestStr
    'MsgBox TestStr
    
    TestStr = Format(2345.25, "$#,###.##") '$2.345.25
    TestLog_ASSERT TestStr = "$2,345.25", "$#,###.##: " & TestStr
    'MsgBox TestStr
    
    TestStr = Format(0.25, "##.###\%") '.25%
    TestLog_ASSERT TestStr = ".25%", "##.###\%: " & TestStr
    'MsgBox TestStr
    
    Exit Sub
errorHandler:
        TestLog_ASSERT (false), testName & ": hit error handler"
End Sub

Sub Custom_Text_Format_Sample()
    Dim myText, TestStr As String
    myText = "VBA"
    
    Dim testName As String
    
    testName = "Test Custom_Text_Format_Sample function"
     On Error GoTo errorHandler
    
    TestStr = Format(myText, "<") 'vba
     TestLog_ASSERT TestStr = "vba", "<: " & TestStr
    'MsgBox TestStr
    
    TestStr = Format("vba", ">") 'VBA
     TestLog_ASSERT TestStr = "VBA", ">: " & TestStr
    'MsgBox TestStr
    
    Exit Sub
errorHandler:
        TestLog_ASSERT (false), testName & "hit error handler"
End Sub



Sub testFormat()
    Dim testName As String
    Dim TestDateTime As Date
    Dim TestStr As String
    testName = "Test Format function"
    
    On Error GoTo errorHandler
    
    TestDateTime = "1/27/2001 5:04:23 PM"
 
    ' Returns the value of TestDateTime in user-defined date/time formats.
    ' Returns "17:4:23".
    TestStr = Format(TestDateTime, "h:m:s")
    TestLog_ASSERT TestStr = "17:4:23", "the format of h:m:s: " & TestStr
    
    ' Returns "05:04:23 PM".
    TestStr = Format(TestDateTime, "ttttt")
    TestLog_ASSERT TestStr = "5:04:23 PM", "the format of ttttt: " & TestStr
    
    ' Returns "Saturday, Jan 27 2001".
    TestStr = Format(TestDateTime, "dddd, MMM d yyyy")
    TestLog_ASSERT TestStr = "Saturday, Jan 27 2001", "the format of dddd, MMM d yyyy: " & TestStr
    
    ' Returns "17:04:23".
    TestStr = Format(TestDateTime, "HH:mm:ss")
    TestLog_ASSERT TestStr = "17:04:23", "the format of HH:mm:ss: " & TestStr
    
    ' Returns "23".
    TestStr = Format(23)
    TestLog_ASSERT TestStr = "23", "no format:" & TestStr
    
    ' User-defined numeric formats.
    ' Returns "5,459.40".
    TestStr = Format(5459.4, "##,##0.00")
    TestLog_ASSERT TestStr = "5,459.40", "the format of ##,##0.00: " & TestStr
    
    ' Returns "334.90".
    TestStr = Format(334.9, "###0.00")
    TestLog_ASSERT TestStr = "334.90", "the format of ###0.00: " & TestStr
    
    ' Returns "500.00%".
    TestStr = Format(5, "0.00%")
    TestLog_ASSERT TestStr = "500.00%", "the format of 0.00%: " & TestStr
    Exit Sub
errorHandler:
        TestLog_ASSERT (false), testName & ": hit error handler"
End Sub

Sub TestLog_ASSERT(assertion As Boolean, Optional testId As String, Optional testComment As String)

    If assertion = True Then
        passCount = passCount + 1
    Else
        Dim testMsg As String
        If Not IsMissing(testId) Then
            testMsg = testMsg + " : " + testId
        End If
        If Not IsMissing(testComment) And Not (testComment = "") Then
            testMsg = testMsg + " (" + testComment + ")"
        End If

        result = result & Chr$(10) & " Failed: " & testMsg
        failCount = failCount + 1
    End If
    
End Sub

