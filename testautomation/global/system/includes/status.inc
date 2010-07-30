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
'*
'* owner : thorsten.bosbach@oracle.com
'*
'* short description : Routines for the status page feature
'*
'\*****************************************************************************************

sub hStatusIn ( sTestAppArea as String, sTestname as String, optional sName as String )
    '///hStatusIn : initilize variables before the teststart
    '///  input  : sTestAppArea => name of the application, to where the test is bound in status database -> gTestAppArea
    '              sTestname    => name of the test (converted to LOWERCASE in this sub!) -> gTestName
    '///+ output : gStatusDuration => starttime of the test
    '///+_       : gTestname    => global name of the test
    '///+_       : gTestAppArea => global name of the tested application as defined in status database
    '///+_       : gTestDate    => global start date of the test ( yyyy-mm-dd )
    '///+_       : gTestTime    => global start time of the test
    ' DEPRECATED: 
    '         sNname       => DEPRECATED just kept for compatibility  
    
    dim bOverRide as boolean
    
    bOverRide = false
    gStatusDuration = now ()                    '(1) used in hStatusOut
                          ' temporarly misused to set the start Date and Time!

    ' Always needed for crashreporter test hint
    gTestName = lcase (sTestname)               '(2)

    ' -------------- EXIT condition ----------------------
    if (NOT isStatusEnabled()) then
        exit sub
    end if

    gTestAppArea = lcase (sTestAppArea)         '(3)
    
    gTestDate = convertDateToDatabase (gStatusDuration)  '(4)
    gTestTime = convertTimeToDatabase (gStatusDuration)  '(5)
    
    ListAllDelete(glsStatusPage())
    gErrorSum = getErrorCount()      ' not 0! if you run 2-times status in/out in one bas-file!
    gWarningSum = getWarningCount()
    gQaErrorSum = getQaErrorCount()
    
    ' make sure we have everything to update the status-Database.
    gDatabasePath = getDatabasePath(privateDatabasePath)
    if ("" = gDatabasePath) then
        'Disable statusfeature, because the public filespace is not available.
        gStatusDatabase = FALSE
        printlog "**  Status will be written to     : DISABLED"
    else
        printlog "**  Status will be written to     : " + gDatabasePath
    endif
    printlog     "**  Test environment preparation  : " + wielange(gTestcaseStart)
end sub

sub hStatusOut ( optional NoKill as Boolean )
    '///hStatusOut : last output for the status-page feature
    '///+ -> create the duration value for the test and call the routine to write the data into the database

    Dim sLocalTestDuration as string

    printlog  ""
    printlog  "**  All tests finished."
    sLocalTestDuration = WieLange( gStatusDuration )
    gTestcaseStart= now ()    ' get time for writing status to database
    
    ' -------------- EXIT condition ----------------------
    ' don't record status if outside of status database
    if (NOT isStatusEnabled()) then
        Printlog "Date: " + Date() + "; Time: " + Time() + "; Duration: " + WieLange ( gStatusDuration )
        exit sub
    end if
    
    if (""=gTestName) then
        warnlog "status.inc::hStatusOut: You forgot to call hStatusIn(''Application'',''FileName.bas'')"
    else
        printlog  "**  Start generating quaste database files."
        hStatusWriteOutputFirstFile() ' write again, to have correct duration written.
        hStatusWriteOutput()
        printlog "**  Creating status duration    : " + wielange(gTestcaseStart)
    end if
    
    PrintLog Chr(13) + "* - End of the test - *"
    Printlog "Date : " + Date() + "   Time: " + Time()
    Printlog "Duration : " + sLocalTestDuration
end sub

sub hStatusAddTestcase()
    ' called from master.inc::TestExit() after every testcase
    ' add to list for second file : testresult table / glsStatusPage()
    ' reset gErrorSum, gWarningSum
    dim sTestcaseDuration as string
    dim sTestcaseStart as string
    Dim sTCname as String
    Dim iCut as Integer
    dim iErrorCount as integer
    dim sErrorList() as string
    dim iQaErrorCount as integer
    dim sQaErrorList() as string
    dim iWarningCount as integer
    dim sWarningList() as string
    Dim sOutput as String
    dim iAllErrorCount as integer
    dim sAllErrorList(42000) as string
    dim i, x as integer
    dim iErrorLevel as integer
    dim sErrorString(4) as string

    '///The entries in the list are ( seperated by TAB ) :
    '///+ testcase name => name of the current testcase in the running test
    '///+ errors => only the errors for the current testcase
    '///+ warnings => only the warnings for the current testcase
    '///+ duration => the duration of the testcase

    sTestcaseDuration = wielange(gTestcaseStart, 1)     '(2)
    sTestcaseStart    = convertDateToDatabase(gTestcaseStart) + " " + convertTimeToDatabase(gTestcaseStart) ' TODO: ask HDE/TBO
    
    sTCname = GetTestcaseName  ' testtool basic command
        iCut = Instr ( sTCname, "(" )
        if (iCut <> 0) then 
            sTCname = Left ( sTCname, iCut - 1 )
        endif
        sTCname = Trim ( sTCname )                      '(1)
    iErrorCount = getErrorCount() - gErrorSum               ' only the errors in a testcase
    iWarningCount = getWarningCount() - gWarningSum         ' only the warnings in a testcase
    iQaErrorCount = getQaErrorCount() - gQaErrorSum         ' only the qaErrors in a testcase

    iAllErrorCount = iErrorCount + iWarningCount + iQaErrorCount
    if (iAllErrorCount > 0) then
        x=1
        sWarningList() = getWarningList()
        for i = (GetWarningCount()+1-iWarningCount) to GetWarningCount()
            sAllErrorList(x) = sWarningList(i)
'd                printlog "++ " + sAllErrorList(x)
            inc(x)
        next i
        sErrorList() = getErrorList()
        for i = (GetErrorCount()+1-iErrorCount) to GetErrorCount()
            sAllErrorList(x) = sErrorList(i)
'd                printlog "++ " + sAllErrorList(x)
            inc(x)
        next i
        sQaErrorList() = getQaErrorList()
        for i = (getQaErrorCount()+1-iQaErrorCount) to getQaErrorCount()
            sAllErrorList(x) = sQaErrorList(i)
'd                printlog "++ " + sAllErrorList(x)
            inc(x)
        next i
    else
        sAllErrorList(0) = "0;0;0;0"
    endif
    
    ' generate status line for testcase and append to global array
    '/// iErrorLevel: 0: no faults; 1: Warning; 2: Error; 3: qaError ///'
    iErrorLevel = -1
'D    printlog "Iall: " + iAllErrorCount + " W:" + iWarningCount + " E: " + iErrorCount
    for i = 0 to iAllErrorCount
        select case i
            case 0: if (0 = iAllErrorCount) then          ' no errors at all
                        iErrorLevel = 0
                    endif
            case 1 to iWarningCount: iErrorLevel = 1      'warnings
            case (iWarningCount +1) to (iWarningCount + iErrorCount): iErrorLevel = 2 ' Errors
            case (iWarningCount + iErrorCount +1) to (iWarningCount + iErrorCount + iQaErrorCount): iErrorLevel = 3 ' qaErrors
        end select
        if (iErrorLevel > -1) then
'd            printlog " " + i + " -------------"
'd            printlog "'" + sAllErrorList(i) + "'"
            sGetErrorStringFields(sAllErrorList(i), sErrorString())
'd            printlog " -------------"
            sOutput =          sTCname _
                    + Chr(9) + sTestcaseDuration _
                    + Chr(9) + iErrorLevel _
                    + Chr(9) + fRemoveLineBreaks(sErrorString(4)) _
                    + Chr(9) + sErrorString(2) _
                    + Chr(9) + trim(sErrorString(3)) _
                    + Chr(9) + fgetFileName(sErrorString(1)) _
                    + Chr(9) 'Description (4)_ 'Line (2)_ 'CVSversion (3)_ 'Filename (1)
            if (sTCname <> "") then
                ListAppend (glsStatusPage(), sOutput)
'd            printlog sOutput
            else
                qaErrorlog "please try not to call a testcase from a testcase #116584#"
            endif
        endif
    next i
    
    ' to set the variables to the current numbers    
    gErrorSum = getErrorCount()
    gQaErrorSum = getQaErrorCount()
    gWarningSum = getWarningCount()
end sub

sub hStatusWriteOutputFirstFile ()
    ' called from hStatusOut and hStatusIn 
    Dim sPlat as String
    Dim sOutFile as String
    Dim sOutFileTemp as String
    Dim sBuildHisPath as string
    Dim sResultPath as string    ' location where to write the files for status to
    Dim i as integer
    dim j as Integer
    dim lTestrun(50) as string
    dim sVersionMajor as string
    dim sVersionMinor as string
    dim sVersionBuilID as string
    dim sDebugInfo as string
    dim sTemp as string
    dim sFileName as string
    dim slVersion() as string
    dim ilVersion as integer
    dim sVersionCWS as string
    dim iPosA as integer
    dim iPosB as integer
    dim bError as boolean
    dim sTestDuration as string
    dim sSource as string
    dim sProduct as string
    dim sUsername as string
    
    '///hStatusWriteOutputFirstFile : output routine for status page of our testscripts
    '///The entries in the list are ( seperated by NEWLINE ) :
    '///+ 1 major => major number of full buildID of StarOffice ( e.g. '642' )
    '///+ 2 minor => minor number of full buildID of StarOffice ( e.g.'L' )
    '///+ 3 buildID => only the buildID of full buildID of StarOffice ( e.g.'7733' )
    '///+ 4 date ( gTestDate ) time ( gTestTime ) => fix date when the test started
    '///+ 5 platform => short cut for platform
    '///+ 6 machine name => name of the PC or UNIX-machine where the test is running
    '///+ 7 user name => E-mail adress of user
    '///+ 8 fileformat => version belonging to this spec
    '///+ 9 language => language of the office
    '///+ 10 test name => name of the test ( e.g. first.bas )
    '///+ 11 test application area (gApplication) => which application is tested
    '///+ 12 test duration => Hours:Minutes:Seconds ( e.g.'01:20:33' )
    '///+ 13 cws name => if it is the master: 'Master' else the name of the childworkspace
    '///+ 14 source tree => 
    '///+ 15 product => 
    '///+ 16 builder => 
    '///+ 17 checksum => 
    '///+ data => 

    if ("unx" = gPlatgroup) then                                  '(5)
        sPlat = gPlatform
    else
        sPlat = "win"
    end if

    sProduct = gProductName                         '(15)

    ' major is from start to 'm'
    iPosA = 1
    iPosB = instr(gVersionsnummer, "m")
    if (iPosB = 0) then ' there is no minor
        iPosB = instr(gVersionsnummer, "(")
    endif    
    sVersionMajor = Mid(gVersionsnummer, iPosA, (iPosB-iPosA))      '(1) Major
    iPosA = iPosB
    iPosB = instr(gVersionsnummer, "(")
    sVersionMinor  = Mid(gVersionsnummer, iPosA, iPosB-iPosA)        '(2) Minor
    iPosA = instr(gVersionsnummer, ":") + 1
    iPosB = instr(gVersionsnummer, ")")
    sVersionBuilID = Mid(gVersionsnummer, iPosA, iPosB-iPosA)        '(3) Build
    if gCWS then
        iPosA = instr(iPosB, gVersionsnummer, ":") + 1
        iPosB = instr(iPosA, gVersionsnummer, "]")
        sVersionCWS = Mid(gVersionsnummer, iPosA, iPosB-iPosA)       '(13) CWS
    else
        sVersionCWS = "Master"
    endif
    sSource = left(gMajor,3)                                       '(14) Source tree
    
    ' for MSC calculation of test duration hh:mm
    sTestDuration = wielange(gStatusDuration, 1)                  '(12)

    if ("" = gReturnAddress) then                               '(7)
        if ("" = gLocalStatusDatabase) then
            warnlog "Please set an e-mail adress for your crashreports in TestTool: Extra->Settings->Crashreport:EMail, it will also be used to send you notifications in case of problems submitting the status of the test to the database (quaste)."
        endif
        sUsername = gUser
    else
        sUsername = gReturnAddress
    endif

    ListAppend ( lTestrun(), "fileformat=0.2"             )
    ListAppend ( lTestrun(), "product=" + sProduct        )
    ListAppend ( lTestrun(), "sourcetree=" + sSource      )
    ListAppend ( lTestrun(), "major=" + sVersionMajor     )
    ListAppend ( lTestrun(), "minor=" + sVersionMinor     )
    ListAppend ( lTestrun(), "buildid=" + sVersionBuilID  )
    ListAppend ( lTestrun(), "oooorigin=" + "")
    ListAppend ( lTestrun(), "startdate=" + gTestDate + " " + gTestTime)         '(4)  ' generated in hStatusIn
    ListAppend ( lTestrun(), "duration=" + sTestDuration  )
    ListAppend ( lTestrun(), "platform=" + sPlat          )
    ListAppend ( lTestrun(), "hostname=" + gPCName        )                        '(6)
    ListAppend ( lTestrun(), "username=" + sUsername      )                        '(7)
    ListAppend ( lTestrun(), "application=" + gTestAppArea)                        '(11) ' generated in hStatusIn
    ListAppend ( lTestrun(), "testname=" + gTestName      )                        '(10) ' generated in hStatusIn
    ListAppend ( lTestrun(), "cws=" + sVersionCWS         )
    ListAppend ( lTestrun(), "ooolanguage=" + iSprache    )                        '(9)
    ListAppend ( lTestrun(), "checksum=" + "")
'    ListAppend ( lTestrun(), "data=" + )
    
    ' files are created at (convertPath'ed):
    sResultPath = convertPath(gDatabasePath)
    sFileName   = fGetQuasteFileName()
    sOutFile    = sResultPath + sFileName
    
    ' TODO: make sure location is writeable! with file 'sOutFile'!!! 

    ' delete old files
    for i = 1 to 4
        sOutFileTemp = sOutFile+i+".txt"
        if (FileExists(sOutFileTemp)) then
'            printlog sOutFileTemp
            kill sOutFileTemp
            if (dir(sOutFileTemp) <> "") then 
                warnLog "OLD File can't get deleted: " + sOutFileTemp
            endif
        end if
    next i

    ListWrite (lTestrun(), sOutFile+"1.txt")
end sub

function fGetQuasteFileName() as string
    dim sPlat as string
    dim sName as string

    if ("unx" = gPlatgroup) then
        sPlat = gPlatform
    else
        sPlat = "win"
    end if

    sName = lcase(sPlat + gUser + gPCname + Left(gTestname, Len(gTestname)-4) + "-" + iSprache + "-" )
    fGetQuasteFileName = removeCharacter(sName,46)  ' remove '.' dots from filename, would result in errors on uploading file.
end function

sub hStatusWriteOutput (optional NoKill as Boolean)
    ' called from hStatusOut    
    Dim sPlat as String
    Dim sOutFile as String
    Dim sOutFileTemp as String
    Dim sBuildHisPath as string
    Dim sResultPath as string    ' location where to write the files for status to
    Dim i as integer
    dim j as Integer
    dim sDebugInfo as string
    dim sTemp as string
    dim sFileName as string
    dim bError as boolean
    dim sTestDuration as string
    
    if ("unx" = gPlatgroup) then                                  '(5)
        sPlat = gPlatform
    else
        sPlat = "win"
    end if

    ' files are created at (convertPath'ed):
    sResultPath = convertPath(gDatabasePath)
    sFileName   = fGetQuasteFileName()
    sOutFile    = sResultPath + sFileName
    
    for i = 1 to ListCount(glsStatusPage())
        glsStatusPage(i) = "data=" + glsStatusPage(i)
    next i
    'write 'testresult' 
    ListWriteAppend (glsStatusPage(), sOutFile+"1.txt")
    if (dir(sOutFile+"1.txt") = "") then warnlog "File wasn't created: " + sOutFile+"1.txt"

    ' debug
' from now on the status routines are not executed, because i use tescases for displaying debug information, that should not get recorded
gTestName="" '!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!    

    ' call php-script to get file into the database
    bError = StatusIntoDatabase (sFileName, sPlat, FALSE, gDatabasePath)
    if bError then
        exit sub ' -> on error no file get's deleted!
    end if

    ' wait until result-file got created; after 3 minutes cancel wait!
    sOutFileTemp = sOutFile+"3.txt"
    i = 0
    listAllDelete(glsStatusPage())
    while (("" = dir(sOutFileTemp)) AND (i < 18))
        sleep 10
        inc (i)
    wend
    bError = True
    if (i = 18) then ' big database error; resultfile wasn't created
        warnlog "Status Write Error! (TimeOut waiting for webservice result)"
    else
        ListRead(glsStatusPage(), sOutFileTemp)   '3
        i = ListCount(glsStatusPage())
        if (1 = i) then
            if (glsStatusPage(1) <> "OK") then
                warnlog "Error in writing status to database (<> OK): '" + glsStatusPage(1) + "'" +chr(13)+ "Email is send to: " + gReturnAddress
            else
                if (gStatusFeatureLevel < 2) then
                    printlog " * - Status successfully written into database - * "
                else
                    printlog " * - Status file successfully created - * "
                endif
                bError = False
            endif
        else
            warnlog "Error in writing status to database (<> 1 line)" +chr(13)+ "Email is send to: " + gReturnAddress
        endif
    endif

    ' delete files
    if (bError=FALSE) then 
            sOutFileTemp = sOutFile+"1.txt"
                try
                    if (dir(sOutFileTemp) <> "") then
                            kill ( sOutFileTemp )
                    end if
                catch
                endcatch
            if (dir (sOutFileTemp) <> "") then 
                warnlog "File wasn't deleted: " + sOutFileTemp
            endif
    endif
    sOutFileTemp = sOutFile+"3.txt"
	try
	    if (dir(sOutFileTemp) <> "") then
		    kill ( sOutFileTemp )
	    end if
	catch
	endcatch
    if (dir (sOutFileTemp) <> "") then 
	warnlog "File wasn't deleted: " + sOutFileTemp
    endif
    sOutFileTemp = sOutFile+"4.htm"
	try
	    if (dir(sOutFileTemp) <> "") then
		    kill ( sOutFileTemp )
	    end if
	catch
	endcatch
    if (dir (sOutFileTemp) <> "") then 
	warnlog "File wasn't deleted: " + sOutFileTemp
    endif
    ListAllDelete (glsStatusPage())   ' delete the list, because if you want to use hStatusIn twice or more
end sub


function StatusIntoDatabase (sFile as String, sPlat as String, NoKill as Boolean, sPath as string) as boolean
    dim sSource as string
    dim sDestination as string
    dim i as integer

    '///StatusIntoDatabase : write the collected data into the database
    if (gStatusFeatureLevel < 2) then
        ' Automatical entry into database
        printlog "**  Calling webservice to grab status file."
        StatusIntoDatabase = getWebPage (sPath, sPath+sFile+"4.htm", sPlat, privateDatabaseServerIP, privateDatabaseServerPath + sFile + "1.txt")
        printlog "**  Waiting for result from webservice."
    else
        ' preparation for manual entry into database
        StatusIntoDatabase = FALSE
        'create 3. file with OK :-)
        sSource = convertPath(sPath)
        sDestination = ""
        if gCWS then
            ' if we have a CWS, generate a string of the CWS name
            i = instr(gVersionsnummer, "[")
            if i > 0 then
                sDestination = Mid(gVersionsnummer, i+1)
                i = len(sDestination)
                sDestination = left(sDestination, i-1)
                i = inStr(sDestination, ":")
                mid(sDestination, i, 1, "_")
             endif
        endif
        sDestination = sSource + lCase(gMajor + gMinor + sDestination)
        TextInDatei("OK", sSource+sFile+"3.txt")
        'move other files to directory, because standard is to delele successfull submitted data
        if (dir(sDestination, 16) = "") then ' doesn't exist      
            MkDir (sDestination)
            if (dir(sDestination, 16) = "") then ' doesn't exist      
                warnlog "Database directory can't get created: '" + sDestination + "'"
            else
                printlog "Database directory created: '" + sDestination + "'"
            endif
        endif
        sDestination = sDestination + gPathSigne
        filecopy(sSource+sFile+"1.txt", sDestination+sFile+"1.txt")
    endif
end function

function getWebPage (sPath as string, sResult as String, sPlat as String, sHost as string, sPage as string) as boolean
    dim iShellReturn as integer

    if (sPlat = "win") then
        sPlat = "exe"
    endif

    try
            'httpSetProxy(Host, Port)
            iShellReturn = httpSend(sHost, sPage, 80, sResult)
    catch
        iShellReturn = 99
    endcatch
        ' when using internal httpSend, iShellReturn contains http status numbers: 200 means: ok
        if (iShellReturn = 99) then 
            printlog "Writing status to database with internal httpsend command failed: " + iShellReturn + chr(13) + sHost+sPage + chr(13) + sResult
        else 
	  if (iShellReturn <> 200) then 
            warnlog "Writing status to database with internal httpsend command failed: " + iShellReturn + chr(13) + sHost+sPage
          endif
        endif
end function


testcase tDebugInfoMysql (sTemp as string)
' to show the debuginfo folded in a testcase (if nokill = true)
    dim fTemp(900) as string

    fTemp(0)=0
    printlog stemp
    try
        ListRead (fTemp(), sTemp)
        for i=1 to ListCount (fTemp())
            if (fTemp(i) <> "") then printlog fTemp(i)
        next i
    catch
    endcatch
endcase

function isStatusEnabled() as boolean
    '/// enable status only when: ///'
    '///+ basedirectory is on server (variable is set to 1 gStatusFeatureLevel) ///'
    isStatusEnabled = gStatusDatabase
end function

function convertDateToDatabase(byVal inDate as Date) as string
    Dim IsoData$, y$, m$, d$

    IsoData$ = CDateToIso (inDate)
    y$ = left$( IsoData$, 4 )
    m$ = mid$( IsoData$, 5, 2 )
    d$ = right$( IsoData$, 2 )
    convertDateToDatabase = y$ + "-" + m$ + "-" + d$
end function

function convertTimeToDatabase(byVal inTime as Date) as string
    dim iSpace as integer
    
    iSpace = inStr(inTime, " ")
    if (iSpace > 0) then
        inTime = right(inTime, len(inTime) - iSpace)
    endif
    if (iSystemSprache = 1) then
        try
            convertTimeToDatabase = TimeValue(inTime)
        catch
            qaErrorLog "global::system::inc::status.inc::convertTimeToDatabase; looking for root cause: 'Data type mismatch'; Input: '" + inTime + "'"
        endcatch
    else
        convertTimeToDatabase = Format (inTime, "hh:mm:ss")
    endif
end function

function getDatabasePath(sSubDirectory as string) as string
    dim sPath as string
    dim sPathSeed as string

    if gStatusFeatureLevel = 2 then
        ' write it below 'errorlog' directory
        sPath = convertPath(GetIniValue (gTesttoolIni, gTTProfileName , "LogBaseDir"))
        if (right(sPath, 1) <> gPathSigne) then
            sPath = sPath + gPathSigne
        end if
        getDatabasePath = sPath
        gLocalStatusDatabase = sPath
    else
        ' assumption: the only supported testcases are always on local fileserver -> gTestToolPath provides a valid volume !
        if (gStatusFeatureLevel = 1) then
            ' global filespace for database is defined in testtoolrc
            sPath = gLocalStatusDatabase + gPathSigne
	    sPath = fRemoveDoubleCharacter(sPath, gPathSigne)
        else
	    'gStatusFeatureLevel = 0
            ' status database server is global defined
            sPath = gTestToolPath + gPathSigne + sSubDirectory
            sPath = convertPath(sPath)
            sPath = fRemoveDoubleCharacter(sPath, gPathSigne)
            sPath = fRelativeToAbsolutePath(sPath)
            sPath = fRemoveDoubleCharacter(sPath+ gPathSigne, gPathSigne)
        endif
        if (NOT fileExists(sPath + "quaste.txt")) then
            qaErrorLog "The public file space seems to be wrong: "  + sPath
        endif
    endif
    getDatabasePath = sPath
end function    

sub sGetErrorStringFields(sIn as string, sOut() as string)
    '/// put semicolon seperated string into an array ///'
    '/// only used on every line from returnvalue of get*List() ///'
    dim sTemp(3) as string
    dim sTemp2() as string
    dim i as integer
    if ("" = sIn) then     ' workaround for i23697 split() returns wrong value on empty string
        for i=0 to 3
            sTemp(i) = ""
        next i
    else
        sTemp() = Split(sIn, ";") 
    endif
    if ((uBound(sTemp())+1) <>  uBound(sOut())) then
        for i = 1 to (uBound(sOut()) -1)
             sOut(i) =  sTemp(i-1)
        next i
        redim sTemp2(uBound(sTemp()) - uBound(sOut())+1) as string
        for i = (uBound(sOut())-1) to uBound(sTemp())
             sTemp2(i-(uBound(sOut())-1)) = sTemp(i)
        next i
        sOut(uBound(sOut())) = join(sTemp2(), ":")
    else
        for i = 0 to uBound(sTemp())
            sOut(i+1) = sTemp(i)
        next i
    endif
    
'    for i = 0 to uBound(sTemp())
'd        printlog "" + i + ": " + sTemp(i)
'    next i
end sub

function fRemoveLineBreaks(sIn as string) as string
    '/// Clean string from reserved characters and remove linebreaks ///'
    '/// only used for errormessage in third field from get*List() ///'
    dim sLocal as string
    dim x as integer
    dim iCharacters(7) as integer
    iCharacters(1) = 9  ' TAB  because it is field seperator in data file
    iCharacters(2) = 10 ' LF   because no linebreak is allowed in data file
    iCharacters(3) = 13 ' CR   because no linebreak is allowed in data file
    iCharacters(4) = 39 ' '    because is string delemiter for mysql        
    iCharacters(5) = 8216 ' '  because is string delemiter for mysql        
    iCharacters(6) = 8217 ' '  because is string delemiter for mysql        
    iCharacters(7) = 92 '\  '  because it is escape code
    
    sLocal = sIn
    
    for x = 1 to 7
        sLocal = removeCharacter(sLocal,iCharacters(x))
    next x
    
    fRemoveLineBreaks = sLocal
end function

function removeCharacter(sIn as string, iCharacter as integer) as string
    dim sLocal as string
    dim sArray() as string
    dim i as integer
    dim iBound as integer
    sLocal = sIn
        if ("" = sLocal) then     ' workaround for i23697 split() returns wrong value on empty string
'            for i=0 to 3
'                sTemp(i) = ""
'            next i
        else
            sArray() = split(sLocal, chr(iCharacter))
        endif
        sLocal = ""
        iBound = uBound(sArray())
'        if (iBound > 0) then printlog "########## " + i + " - " + iCharacters(x) + " ++++ " + iBound 
        for i = 0 to iBound
            sLocal = sLocal + sArray(i)
        next i
    removeCharacter = sLocal
end function

function fgetFileName(byVal sIn as string) as string
    '/// extract file name from string, where PathSeperator is always Backslash ///'
    '/// only used for filestring in first field from get*List() ///'
    dim sTemp(0) as string
    if ("" = sIn) then     ' workaround for i23697 split() returns wrong value on empty string
        sTemp(0) = ""
    else
        sTemp() = split(sIn, "\") ' GH returns hopefully always a Backslash as seperator
    endif
    fgetFileName = sTemp(uBound(sTemp()))
end function

