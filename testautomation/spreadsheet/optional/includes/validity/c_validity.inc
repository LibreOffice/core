'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
' DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'
' Copyright 2000, 2010 Oracle and/or its affiliates.
'
' OpenOffice.org - a multi-platform office productivity suite
'
' This file is part of OpenOffice.org.
'
' OpenOffice.org is free software: you can redistribute it and/or modify
' it under the terms of the GNU Lesser General Public License version 3
' only, as published by the Free Software Foundation.
'
' OpenOffice.org is distributed in the hope that it will be useful,
' but WITHOUT ANY WARRANTY; without even the implied warranty of
' MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
' GNU Lesser General Public License version 3 for more details
' (a copy is included in the LICENSE file that accompanied this code).
'
' You should have received a copy of the GNU Lesser General Public License
' version 3 along with OpenOffice.org.  If not, see
' <http://www.openoffice.org/license.html>
' for a copy of the LGPLv3 License.
'
'/************************************************************************
'**
'** owner : oliver.craemer@Sun.COM
'**
'** short description :  Complete test for data validity
'**
'\***********************************************************************

testcase tValidity_Check_Message

    printlog "This test checks, if an error messgae appears, when a wrong content is inserted into the appropriate cells"

    dim sInputFile as string
    dim sOutputFile as string
    dim sMType as string
    dim sInfoStr as string
    dim iCount as integer

    sInputFile = convertpath(gTesttoolPath & "spreadsheet/optional/input/validity.ods")
    sOutputFile = convertpath (gOfficepath & "user/work/validity.ods.ods)"

    printlog " Open document  <i>gTesttoolPath</i>spreadsheet/optional/input/validity.ods"
    call hFileOpen (sInputFile)
    printlog " Save the document under <i>gOfficePath</i>user/work/validity.ods"
    if NOT hFileSaveAsWithFilter ( sOutputFile, "calc8" ) then
        warnlog "Saving test document localy failed -> Aborting"
        call hCloseDocument
        goto endsub
    endif

    printlog " Do the following operations (see values in table) with every of the 4 sheets."
    printlog " Goto first cell and insert the correct value"
    printlog " Press RETURN"
    printlog "  No warning or information message should appear"
    printlog " Enter the incorrect value"
    printlog " Press RETURN"
    printlog "  A warning or information message should appear"
    for iCount = 1 to 4
        select case iCount
        case 1 : sMType = "Stop"          : sInfoStr = "First"
        case 2 : sMType = "Warning"       : sInfoStr = "Second"
        case 3 : sMType = "Information"   : sInfoStr = "Third"
        case 4 : sMType = "Macro"         : sInfoStr = "Macro"
        end select

        call fCalcSelectSheet(sMType)
        printlog " *** " & sInfoStr & " it is checked, if the " & sMType & " messages appears, if wrong contents are inserted into the appropriate cells"
        '///+<UL>
        '///+<LI>Goto first cell and insert the <i>correct value</i></LI>
        '///+<LI>Press RETURN</LI>
        '///+<LI><UL><LI>No warning or information message should appear</LI></UL></LI>
        '///+<LI>Enter the <i>incorrect value</I>
        '///+<LI>Press RETURN</LI>
        '///+<LI><UL><LI>A warning or information message should appear</LI></UL></LI>
        '///+<LI><TABLE BORDER=1>
        '///+ <TR><TH>Cell address</TH><TH>correct value</TH><TH>incorrect value</TH></TR>
        '///+ <TR><TD>B4</TD><TD>3</TD><TD>8</TD></TR>
        call check_message(sMType, "B4", "3", "8")
        '///+ <TR><TD>B5</TD><TD>9</TD><TD>10</TD></TR>
        call check_message(sMType, "B5", "9", "10")
        '///+ <TR><TD>B6</TD><TD>28</TD><TD>25</TD></TR>
        call check_message(sMType, "B6", "28", "25")
        '///+ <TR><TD>B7</TD><TD>10</TD><TD>12</TD></TR>
        call check_message(sMType, "B7", "10", "12")
        '///+ <TR><TD>B8</TD><TD>11</TD><TD>9</TD></TR>
        call check_message(sMType, "B8", "11", "9")
        '///+ <TR><TD>B9</TD><TD>5</TD><TD>3</TD></TR>
        call check_message(sMType, "B9", "5", "3")
        '///+ <TR><TD>B10</TD><TD>1</TD><TD>6</TD></TR>
        call check_message(sMType, "B10", "1", "6")
        '///+ <TR><TD>B11</TD><TD>1</TD><TD>6</TD></TR>
        call check_message(sMType, "B11", "1", "6")
        '///+ <TR><TD>B17</TD><TD>3,14</TD><TD>3,15</TD></TR>
        call check_message(sMType, "B17", "3,14", "3,15")
        '///+ <TR><TD>B18</TD><TD>10,14</TD><TD>10,16</TD></TR>
        call check_message(sMType, "B18", "10,14", "10,16")
        '///+ <TR><TD>B19</TD><TD>25,10</TD><TD>25,09</TD></TR>
        call check_message(sMType, "B19", "25,10", "25,09")
        '///+ <TR><TD>B20</TD><TD>10,5</TD><TD>10,7</TD></TR>
        call check_message(sMType, "B20", "10,5", "10,7")
        '///+ <TR><TD>B21</TD><TD>11,9</TD><TD>10</TD></TR>
        call check_message(sMType, "B21", "11,9", "10")
        '///+ <TR><TD>B22</TD><TD>3,145</TD><TD>3,14</TD></TR>
        call check_message(sMType, "B22", "3,145", "3,14")
        '///+ <TR><TD>B23</TD><TD>1,45</TD><TD>1,38</TD></TR>
        call check_message(sMType, "B23", "1,45", "1,38")
        '///+ <TR><TD>B24</TD><TD>2,03</TD><TD>2,15</TD></TR>
        call check_message(sMType, "B24", "2,03", "2,15")
        '///+ <TR><TD>E4</TD><TD>13.12.99</TD><TD>13.11.99</TD></TR>
        call check_message(sMType, "E4", "13.12.99", "13.11.99")
        '///+ <TR><TD>E5</TD><TD>12.12.99</TD><TD>13.12.99</TD></TR>
        call check_message(sMType, "E5", "12.12.99", "13.12.99")
        '///+ <TR><TD>E6</TD><TD>14.12.99</TD><TD>13.12.99</TD></TR>
        call check_message(sMType, "E6", "14.12.99", "13.12.99")
        '///+ <TR><TD>E7</TD><TD>13.12.99</TD><TD>14.12.99</TD></TR>
        call check_message(sMType, "E7", "13.12.99", "14.12.99")
        '///+ <TR><TD>E8</TD><TD>13.12.99</TD><TD>12.12.99</TD></TR>
        call check_message(sMType, "E8", "13.12.99", "12.12.99")
        '///+ <TR><TD>E9</TD><TD>13.11.99</TD><TD>13.12.99</TD></TR>
        call check_message(sMType, "E9", "13.11.99", "13.12.99")
        '///+ <TR><TD>E10</TD><TD>23.12.99</TD><TD>12.12.99</TD></TR>
        call check_message(sMType, "E10", "23.12.99", "12.12.99")
        '///+ <TR><TD>E11</TD><TD>24.12.99</TD><TD>23.12.99</TD></TR>
        call check_message(sMType, "E11", "24.12.99", "23.12.99")
        '///+ <TR><TD>E17</TD><TD>13:15</TD><TD>13:16</TD></TR>
        call check_message(sMType, "E17", "13:15", "13:16")
        '///+ <TR><TD>E18</TD><TD>10:15</TD><TD>14:16</TD></TR>
        call check_message(sMType, "E18", "10:15", "14:16")
        '///+ <TR><TD>E19</TD><TD>23:10</TD><TD>12:09</TD></TR>
        call check_message(sMType, "E19", "23:10", "12:09")
        '///+ <TR><TD>E20</TD><TD>13:15</TD><TD>13:16</TD></TR>
        call check_message(sMType, "E20", "13:15", "13:16")
        '///+ <TR><TD>E21</TD><TD>13:15</TD><TD>13:14</TD></TR>
        call check_message(sMType, "E21", "13:15", "13:14")
        '///+ <TR><TD>E22</TD><TD>13:16</TD><TD>13:15</TD></TR>
        call check_message(sMType, "E22", "13:16", "13:15")
        '///+ <TR><TD>E23</TD><TD>13:15</TD><TD>23:13</TD></TR>
        call check_message(sMType, "E23", "13:15", "23:13")
        '///+ <TR><TD>E24</TD><TD>12:59</TD><TD>13:00</TD></TR>
        call check_message(sMType, "E24", "12:59", "13:00")
        '///+ <TR><TD>H4</TD><TD>abc</TD><TD>abcd</TD></TR>
        call check_message(sMType, "H4", "abc", "abcd")
        '///+ <TR><TD>H5</TD><TD>abcdefgrd</TD><TD>aderfgtzjm</TD></TR>
        call check_message(sMType, "H5", "abcdefgrd", "aderfgtzjm")
        '///+ <TR><TD>H6</TD><TD>rftgzhujikolmnbvcxyaswqepk</TD><TD>ftgzhnujikm,lopredswqayx</TD></TR>
        call check_message(sMType, "H6", "rftgzhujikolmnbvcxyaswqepk", "ftgzhnujikm,lopredswqayx")
        '///+ <TR><TD>H7</TD><TD>asweqrfvbg</TD><TD>edcvfrtgbnh</TD></TR>
        call check_message(sMType, "H7", "asweqrfvbg", "edcvfrtgbnh")
        '///+ <TR><TD>H8</TD><TD>edcswxyaqtgb</TD><TD>ujnmkipztr</TD></TR>
        call check_message(sMType, "H8", "edcswxyaqtgb", "ujnmkipztr")
        '///+ <TR><TD>H9</TD><TD>sdxc</TD><TD>tgb</TD></TR>
        call check_message(sMType, "H9", "sdxc", "tgb")
        '///+ <TR><TD>H10</TD><TD>rfvbj</TD><TD>dfcvgb</TD></TR>
        call check_message(sMType, "H10", "rfvbj", "dfcvgb")
        '///+ <TR><TD>H11</TD><TD>g</TD><TD>we</TD></TR>
        call check_message(sMType, "H11", "g", "we")
        '///+ <TR><TH>New featurs OpenOffice 2.0</TH></TR>
        '///+ <TR><TD>B30</TD><TD>February</TD><TD>January</TD></TR>
        call check_message(sMType, "B30", "February", "January")
        '///+ <TR><TD>B31</TD><TD>May</TD><TD>February</TD></TR>
        call check_message(sMType, "B31", "May", "February")
        '///+ <TR><TD>B32</TD><TD>June</TD><TD>January</TD></TR>
        call check_message(sMType, "B32", "June", "January")
        '///+ <TR><TD>B33</TD><TD>August</TD><TD>November</TD></TR>
        call check_message(sMType, "B33", "August", "November")
        '///+ <TR><TD>B34</TD><TD>September</TD><TD>December</TD></TR>
        call check_message(sMType, "B34", "September", "December")
        '///+ <TR><TD>B35</TD><TD>October</TD><TD>March</TD></TR>
        call check_message(sMType, "B35", "October", "March")
        '///+ <TR><TD>B36</TD><TD>July</TD><TD>August</TD></TR>
        call check_message(sMType, "B36", "July", "August")
        '///+ <TR><TD>B37</TD><TD>April</TD><TD>October</TD></TR>
        call check_message(sMType, "B37", "April", "October")
        '///+ <TR><TD>B38</TD><TD>Name03</TD><TD>Name07</TD></TR>
        call check_message(sMType, "B38", "Name03", "Name07")
        '///+ <TR><TD>E30</TD><TD>alpha</TD><TD>omega</TD></TR>
        call check_message(sMType, "E30", "alpha", "omega")
        '///+ <TR><TD>E31</TD><TD>beta</TD><TD>zeta</TD></TR>
        call check_message(sMType, "E31", "beta", "zeta")
        '///+ <TR><TD>E32</TD><TD>gamma</TD><TD>psi</TD></TR>
        call check_message(sMType, "E32", "gamma", "psi")
        '///+ <TR><TD>E33</TD><TD>delta</TD><TD>pi/2</TD></TR>
        call check_message(sMType, "E33", "delta", "pi/2")
        '///+ </TABLE></LI></UL>
    next iCount
    '/// Close the document
    printlog "Close the document"
    call hCloseDocument

endcase
'
'-------------------------------------------------------------------------------
'
testcase tValidity_Check_Excel
    '/// This test checks, if an error messgae appears, when a wrong content is inserted into the appropriate cells

    dim sInputFile as String
    dim sOutputFile as String
    dim sMType as String
    dim iCount as integer
    dim sInfoStr as String
    dim sdecimalseperator as string
    sDecimalseperator = GetDecimalSeperator

    sInputFile = convertpath(gTesttoolPath & "spreadsheet/optional/input/validity.ods")
    sOutputFile = convertpath(gOfficePath & "user/work/validity.xls")

    '/// Open <i>gTesttoolPath</i>/calc/special/input/validity.ods
    printlog "Open <i>gTesttoolPath</i>/calc/special/input/validity.ods"
    call hFileOpen (sInputFile)
    Kontext "SecurityWarning"
    '/// If security warning occours click on 'Enable Macros'.
    if SecurityWarning.exists(2) then
        SecurityWarning.OK
    endif
    sleep (2)
    '/// Save the document under <i>gOfficePath</i>user/work/validity.xls (MS Excel 97 format)
    if NOT hFileSaveAsWithFilterKill ( sOutputFile, "MS Excel 97" ) then
        warnlog "Saving test document localy failed -> Aborting"
        call hCloseDocument
        goto endsub
    endif
    sleep (2)
    '/// Close the document
    call hCloseDocument
    sleep (2)
    '/// Open the exported document
    call hFileOpen ( sOutputFile )
    sleep (2)
    printlog " Do the following operations (see values in table) with every of the 3 sheets."
    printlog " Goto first cell and insert the correct value"
    printlog " Press RETURN"
    printlog "  No warning or information message should appear"
    printlog " Enter the incorrect value"
    printlog " Press RETURN"
    printlog "  A warning or information message should appear"
    for iCount = 1 to 3
        select case iCount
        case 1 : sMType = "Stop"          : sInfoStr = "First"
        case 2 : sMType = "Warning"       : sInfoStr = "Second"
        case 3 : sMType = "Information"   : sInfoStr = "Third"
        end select
        '/// Do the following operations (see values in table) with every of the 3 sheets.
        call fCalcSelectSheet(sMType)
        printlog " *** " & sInfoStr & " it is checked, if the " & sMType & " messages appears, if wrong contents are " & "inserted into the appropriate cells"
        '///+<UL>
        '///+<LI>Goto first cell and insert the <i>correct value</i></LI>
        '///+<LI>Press RETURN</LI>
        '///+<LI><UL><LI>No warning or information message should appear</LI></UL></LI>
        '///+<LI>Enter the <i>incorrect value</I>
        '///+<LI>Press RETURN</LI>
        '///+<LI><UL><LI>A warning or information message should appear</LI></UL></LI>
        '///+<LI><TABLE BORDER=1>
        '///+ <TR><TH>Cell address</TH><TH>correct value</TH><TH>incorrect value</TH></TR>
        '///+ <TR><TD>B4</TD><TD>3</TD><TD>8</TD></TR>
        call check_message(sMType, "B4", "3", "8")
        '///+ <TR><TD>B5</TD><TD>9</TD><TD>10</TD></TR>
        call check_message(sMType, "B5", "9", "10")
        '///+ <TR><TD>B6</TD><TD>28</TD><TD>25</TD></TR>
        call check_message(sMType, "B6", "28", "25")
        '///+ <TR><TD>B7</TD><TD>10</TD><TD>12</TD></TR>
        call check_message(sMType, "B7", "10", "12")
        '///+ <TR><TD>B8</TD><TD>11</TD><TD>9</TD></TR>
        call check_message(sMType, "B8", "11", "9")
        '///+ <TR><TD>B9</TD><TD>5</TD><TD>3</TD></TR>
        call check_message(sMType, "B9", "5", "3")
        '///+ <TR><TD>B10</TD><TD>1</TD><TD>6</TD></TR>
        call check_message(sMType, "B10", "1", "6")
        '///+ <TR><TD>B11</TD><TD>1</TD><TD>6</TD></TR>
        call check_message(sMType, "B11", "1", "6")
        '///+ <TR><TD>B17</TD><TD>3,14</TD><TD>3,15</TD></TR>
        call check_message(sMType, "B17", "3" & sdecimalseperator & "14", "3" & sdecimalseperator & "15")
        '///+ <TR><TD>B18</TD><TD>10,14</TD><TD>10,16</TD></TR>
        call check_message(sMType, "B18", "10" & sdecimalseperator & "14", "10" & sdecimalseperator & "16")
        '///+ <TR><TD>B19</TD><TD>25,10</TD><TD>25,09</TD></TR>
        call check_message(sMType, "B19", "25" & sdecimalseperator & "10", "25" & sdecimalseperator & "09")
        '///+ <TR><TD>B20</TD><TD>10,5</TD><TD>10,7</TD></TR>
        call check_message(sMType, "B20", "10" & sdecimalseperator & "5", "10" & sdecimalseperator & "7")
        '///+ <TR><TD>B21</TD><TD>11,9</TD><TD>10</TD></TR>
        call check_message(sMType, "B21", "11" & sdecimalseperator & "9", "10")
        '///+ <TR><TD>B22</TD><TD>3,145</TD><TD>3,14</TD></TR>
        call check_message(sMType, "B22", "3" & sdecimalseperator & "145", "3" & sdecimalseperator & "14")
        '///+ <TR><TD>B23</TD><TD>1,45</TD><TD>1,38</TD></TR>
        call check_message(sMType, "B23", "1" & sdecimalseperator & "45", "1" & sdecimalseperator & "38")
        '///+ <TR><TD>B24</TD><TD>2,03</TD><TD>2,15</TD></TR>
        call check_message(sMType, "B24", "2" & sdecimalseperator & "03", "2" & sdecimalseperator & "15")
        '///+ <TR><TD>E4</TD><TD>13.12.99</TD><TD>13.11.99</TD></TR>
        call check_message(sMType, "E4", "13.12.99", "13.11.99")
        '///+ <TR><TD>E5</TD><TD>12.12.99</TD><TD>13.12.99</TD></TR>
        call check_message(sMType, "E5", "12.12.99", "13.12.99")
        '///+ <TR><TD>E6</TD><TD>14.12.99</TD><TD>13.12.99</TD></TR>
        call check_message(sMType, "E6", "14.12.99", "13.12.99")
        '///+ <TR><TD>E7</TD><TD>13.12.99</TD><TD>14.12.99</TD></TR>
        call check_message(sMType, "E7", "13.12.99", "14.12.99")
        '///+ <TR><TD>E8</TD><TD>13.12.99</TD><TD>12.12.99</TD></TR>
        call check_message(sMType, "E8", "13.12.99", "12.12.99")
        '///+ <TR><TD>E9</TD><TD>13.11.99</TD><TD>13.12.99</TD></TR>
        call check_message(sMType, "E9", "13.11.99", "13.12.99")
        '///+ <TR><TD>E10</TD><TD>23.12.99</TD><TD>12.12.99</TD></TR>
        call check_message(sMType, "E10", "23.12.99", "12.12.99")
        '///+ <TR><TD>E11</TD><TD>24.12.99</TD><TD>23.12.99</TD></TR>
        call check_message(sMType, "E11", "24.12.99", "23.12.99")
        '///+ <TR><TD>E17</TD><TD>13:15</TD><TD>13:16</TD></TR>
        call check_message(sMType, "E17", "13:15", "13:16")
        '///+ <TR><TD>E18</TD><TD>10:15</TD><TD>14:16</TD></TR>
        call check_message(sMType, "E18", "10:15", "14:16")
        '///+ <TR><TD>E19</TD><TD>23:10</TD><TD>12:09</TD></TR>
        call check_message(sMType, "E19", "23:10", "12:09")
        '///+ <TR><TD>E20</TD><TD>13:15</TD><TD>13:16</TD></TR>
        call check_message(sMType, "E20", "13:15", "13:16")
        '///+ <TR><TD>E21</TD><TD>13:15</TD><TD>13:15</TD></TR>
        call check_message(sMType, "E21", "13:15", "13:14")
        '///+ <TR><TD>E22</TD><TD>13:16</TD><TD>13:15</TD></TR>
        call check_message(sMType, "E22", "13:16", "13:15")
        '///+ <TR><TD>E23</TD><TD>13:15</TD><TD>23:13</TD></TR>
        call check_message(sMType, "E23", "13:15", "23:13")
        '///+ <TR><TD>E24</TD><TD>12:59</TD><TD>13:00</TD></TR>
        call check_message(sMType, "E24", "12:59", "13:00")
        '///+ <TR><TD>H4</TD><TD>abc</TD><TD>abcd</TD></TR>
        call check_message(sMType, "H4", "abc", "abcd")
        '///+ <TR><TD>H5</TD><TD>abcdefgrd</TD><TD>aderfgtzjm</TD></TR>
        call check_message(sMType, "H5", "abcdefgrd", "aderfgtzjm")
        '///+ <TR><TD>H6</TD><TD>rftgzhujikolmnbvcxyaswqepk</TD><TD>ftgzhnujikm,lopredswqayx</TD></TR>
        call check_message(sMType, "H6", "rftgzhujikolmnbvcxyaswqepk", "ftgzhnujikm,lopredswqayx")
        '///+ <TR><TD>H7</TD><TD>asweqrfvbg</TD><TD>edcvfrtgbnh</TD></TR>
        call check_message(sMType, "H7", "asweqrfvbg", "edcvfrtgbnh")
        '///+ <TR><TD>H8</TD><TD>edcswxyaqtgb</TD><TD>ujnmkipztr</TD></TR>
        call check_message(sMType, "H8", "edcswxyaqtgb", "ujnmkipztr")
        '///+ <TR><TD>H9</TD><TD>sdxc</TD><TD>tgb</TD></TR>
        call check_message(sMType, "H9", "sdxc", "tgb")
        '///+ <TR><TD>H10</TD><TD>rfvbj</TD><TD>dfcvgb</TD></TR>
        call check_message(sMType, "H10", "rfvbj", "dfcvgb")
        '///+ <TR><TD>H11</TD><TD>g</TD><TD>we</TD></TR>
        call check_message(sMType, "H11", "g", "we")
        '///+ <TR><TH>New featurs OpenOffice 2.0</TH></TR>we</TD></TR>
        '///+ <TR><TD>B30</TD><TD>February</TD><TD>January</TD></TR>
        call check_message(sMType, "B30", "February", "January")
        '///+ <TR><TD>B31</TD><TD>May</TD><TD>February</TD></TR>
        call check_message(sMType, "B31", "May", "February")
        '///+ <TR><TD>B32</TD><TD>June</TD><TD>January</TD></TR>
        call check_message(sMType, "B32", "June", "January")
        '///+ <TR><TD>B33</TD><TD>August</TD><TD>November</TD></TR>
        call check_message(sMType, "B33", "August", "November")
        '///+ <TR><TD>B34</TD><TD>September</TD><TD>December</TD></TR>
        call check_message(sMType, "B34", "September", "December")
        '///+ <TR><TD>B35</TD><TD>October</TD><TD>March</TD></TR>
        call check_message(sMType, "B35", "October", "March")
        '///+ <TR><TD>B36</TD><TD>July</TD><TD>August</TD></TR>
        call check_message(sMType, "B36", "July", "August")
        '///+ <TR><TD>B37</TD><TD>April</TD><TD>October</TD></TR>
        call check_message(sMType, "B37", "April", "October")
        '///+ <TR><TD>B38</TD><TD>Name03</TD><TD>Name07</TD></TR>
        call check_message(sMType, "B38", "Name03", "Name07")
        '///+ <TR><TD>E30</TD><TD>alpha</TD><TD>omega</TD></TR>
        call check_message(sMType, "E30", "alpha", "omega")
        '///+ <TR><TD>E31</TD><TD>beta</TD><TD>zeta</TD></TR>
        call check_message(sMType, "E31", "beta", "zeta")
        '///+ <TR><TD>E32</TD><TD>gamma</TD><TD>psi</TD></TR>
        call check_message(sMType, "E32", "gamma", "psi")
        '///+ <TR><TD>E33</TD><TD>delta</TD><TD>pi/2</TD></TR>
        call check_message(sMType, "E33", "delta", "pi/2")
        '///+ </TABLE></LI></UL>
    next iCount
    '/// File / Close
    call hCloseDocument
endcase

