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
'* owner : helge.delfs@oracle.com
'*
'* short description : Global Tools II
'*
'\*************************************************************************************

function hSetLocaleStrings (fLocale as String, TBOstringLocale() as String ) as Boolean
'TODO: JSI, make real description from it!
' creator: TBO @ 25.10.2001
'/// function to set a string array with language dependant strings ///
'/// format of file (fLocale): ///
'///+ 1.line: entries/lines per language => x ///
'///+ 2.line: first language (A) number (iSprache) ///
'///+ 3.line: 1. string language A ///
'///+ 4.lin3: 2.language string  A ///
'///+ ... ///
'///+ (((x+1)*1) +2).line second language (B) number ///
'///+ (((x+1)*1) +2)+1.line:  1. string language B ///
'///+ ... ///
'///+ example file @ "input\\writer\\la_sp\\locale.txt" ///'
'
'/// the function parses the file until it finds the language (iSprache) or until EOF ///
'///+ on success the variable from th ecalling argument ///
'///+ gets set, ///
   dim lLocale (15*20) as string ' list, where file gets loaded into
   dim i,y,x as integer
   dim bFoundLanguage as Boolean
   hSetLocaleStrings = FALSE
   lLocale(0)=0
   fLocale = ConvertPath(fLocale)
   if ListRead (lLocale (), fLocale, "UTF8" ) then
'      printlog "LOCALE: read file :-)"

      bFoundLanguage = FALSE
  ' check file format
      if ( (ListCount(lLocale ()) -1) mod (val(lLocale (1))+1) ) <> 0 then
         warnlog "file has wrong format :-( : lines: "+ ListCount(lLocale ()) +", lenght of entries: "+ lLocale (1) +", (lenght -1) modulo lenghtOfEntries: "+ ( ListCount(lLocale ()) -1) mod ( val(lLocale (1)) +1 )
      else
      '                ( all lines in file  )          (trnsl words)
         for i=0 to ( ( (ListCount(lLocale ())-1) / (val(lLocale (1))+1) )-1)
           '    ( (val(lLocale (1))+1) *i+2)
            x = ( (val(lLocale (1))  ) *i+2 +i) ' line number of entry language
  ' print every language found:
'            printlog "position: "+i+" @ line: "+x+" Language: "+lLocale (x)
  ' check if at suspected language number position is a number
            if (val(lLocale (x)) > 0) then
  ' set string variable if it is the right language
               if (iSprache = val(lLocale (x))) then
'                  printlog "          ^ LOCALE: found needed language :-)"
                  for y=1 to val(lLocale (1))
                     TBOstringLocale(y) = lLocale (x+y)
                     if (TBOstringLocale(y) = "") then
                         qaErrorLog("missing string: " + y + ": '" + lLocale (2+y) + "'")
                     endif
                  next y
                  bFoundLanguage = TRUE
               endif
            else
               warnlog "LOCALE: this is no number :-( FileFormatError"
            end if
         next i
         if (bFoundLanguage = FALSE) then
            qaErrorLog "LOCALE: please add language to LOCALE file!: "+ iSprache
         endif
      endif
   else
      warnlog "LOCALE: file doesn't exist :-( : "+fLocale
   endif
   hSetLocaleStrings = bFoundLanguage
end function

function dec(Ref as integer)
'/// decrement variable, call it like 'dec variable' ///'
' reference or value .-) an excursion :-))
' to give this func a var as ref: call without ANY brackets => 'dec Variable'
' opposite of this to call it via value ! WE DON'T WANT THIS !
' (would be 'dec (Variable)' or in declaration 'function dec (ByVal x)')
   Ref = Ref - 1
end function

'-------------------------------------------------------------------------

function inc(Ref as integer)
'/// increment variable, call it like 'dec variable' ///'
   Ref = Ref + 1
end function

'-------------------------------------------------------------------------

function fSetMeasurementToCM() as string
'/// Sets the measurement unit to centimeter (cm) and returns the unit.
    Dim i as integer

    const LANGUAGE_CODE_JAPANESE = 81
    
    Call hNewDocument
    ToolsOptions
        select case UCase(gApplication)
            case "WRITER"
            Call hToolsOptions("WRITER","GENERAL")
                                                Masseinheit.Select(2)
                                                if iSprache = LANGUAGE_CODE_JAPANESE then
                                                    fSetMeasurementToCM = Right$( Tabulatorenabstand.Gettext, 3 )        
                                                else
                                                    fSetMeasurementToCM = Right$( Tabulatorenabstand.Gettext, 2 )        
                                                endif            
            case "CALC"
            Call hToolsOptions("CALC","GENERAL")
                                                Masseinheit.Select(2)
                                                if iSprache = LANGUAGE_CODE_JAPANESE then
                                                    fSetMeasurementToCM = Right$( Tabulator.Gettext , 3 )        
                                                else
                                                    fSetMeasurementToCM = Right$( Tabulator.Gettext , 2 )        
                                                endif
            case "IMPRESS"
            Call hToolsOptions("IMPRESS","GENERAL")
                                                Masseinheit.Select(2)
                                                if iSprache = LANGUAGE_CODE_JAPANESE then
                                                    fSetMeasurementToCM = Right$( Tabulatorabstand.Gettext, 3 )        
                                                else
                                                    fSetMeasurementToCM = Right$( Tabulatorabstand.Gettext, 2 )        
                                                endif            
            case "DRAW"
            Call hToolsOptions("DRAW","GENERAL")
                                                Masseinheit.Select(2)
                                                if iSprache = LANGUAGE_CODE_JAPANESE then
                                                    fSetMeasurementToCM = Right$( Tabulatorabstand.Gettext, 3 )        
                                                else
                                                    fSetMeasurementToCM = Right$( Tabulatorabstand.Gettext, 2 )        
                                                endif                        
            case "MASTERDOCUMENT"
            Call hToolsOptions("WRITER","GENERAL")
                                                Masseinheit.Select(2)
                                                if iSprache = LANGUAGE_CODE_JAPANESE then
                                                    fSetMeasurementToCM = Right$( Tabulatorenabstand.Gettext, 3 )        
                                                else
                                                    fSetMeasurementToCM = Right$( Tabulatorenabstand.Gettext, 2 )        
                                                endif                        
            case "HTML"
            Call hToolsOptions("HTML","VIEW")
                                                Masseinheit.Select(2)
                                                'in Writer/Web also the Writer has to be set to cm 
                                                'because .sdw, .sxw etc. export to HTML depends on it.
                                                Call hToolsOptions("WRITER","GENERAL")
                                                Masseinheit.Select(2)
                                                if iSprache = LANGUAGE_CODE_JAPANESE then
                                                    fSetMeasurementToCM = Right$( Tabulatorenabstand.Gettext, 3 )        
                                                else
                                                    fSetMeasurementToCM = Right$( Tabulatorenabstand.Gettext, 2 )        
                                                endif            
            case else                         : warnlog swhereIsThisFunction & "For this module ("& gApplication &") no decimal seperator setting exists."
        end select                   
    printlog "Info: Measurement unit has been set to centimeters."
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK
    Call hCloseDocument
end function

'-------------------------------------------------------------------------

function fRemoveDoubleCharacter(stringToChange as string, sCharacter as string) As String   
'/// Removes every Character' after a 'Character' in a given string.   
   Dim lLength, n As Long
   Dim sNextLetter As String
   Dim sLastLetter As String
   Dim sFinalString As String
   Dim sTxt, sChar As String

    'store all arguments in
   sTxt = stringToChange
   lLength = Len(sTxt)
   sLastLetter = left(sTxt, 1)
   sFinalString = sLastLetter

   For n = 2 To lLength Step 1
      sNextLetter = Mid(sTxt, n, 1)
      If (sCharacter+sCharacter <> sLastLetter + sNextLetter) Then
         sFinalString = sFinalString + sNextLetter
      End If
      sLastLetter = sNextLetter
   Next n
   fRemoveDoubleCharacter = sFinalString
End Function

'-------------------------------------------------------------------------

function writeCrashRepFile()
'/// Creates a file <i>(gOfficePath)</i>/user/work/crashrep.txt with two lines:
'///+ <ol><li>name of .bas file</li>
'///+ <li>name of testcase</li></ol>  
    Dim sFile as string
    Dim sContent(5) as string
    
    sFile = ConvertPath (gOfficePath + "user\work\crashrep.txt")
    listAppend(sContent(), gTestName) ' get's set in hStatusIn()
    listAppend(sContent(), getTestcaseName)
    listWrite(sContent(), sFile)
end function

'-------------------------------------------------------------------------

function hGetUNOService(optional bSilent as boolean, optional byRef sUnoPortExternal as string) as object
'/// Function enables the UNO communication inside the
'///+ TestTool to the office application.
'/// INPUT: optional <i>bSilent</i> to suppress informal messages, but no warnings
'/// INPUT: optional <i>sUnoPortExternal</i> to just get the UNO port number passed to that variable
    Dim sResultUno as string
    Dim sUnoPort as string
    Dim sOfficeParameters as string
    Dim sUnoOffice as string
    Dim bJustGettingPort as boolean
    Dim sTTPort as string
    
    ' To not to change the old behaviour, set variable if parameter is not given
    if (isMissing(bSilent)) then
        bSilent = TRUE
    end if

    ' master.inc::sStartUpOffice needs just the Port Numberr from UNO, to pass it to first start up
    if (isMissing(sUnoPortExternal)) then
        bJustGettingPort = FALSE
    else
        bJustGettingPort = TRUE
        sUnoPortExternal = "" ' clear it 
    end if
     
    'To enable spaces and special chars in path;
    'This doesn't work for the TestTool command 'start'
    'But for the 'shell' command it is ok.
    sUnoOffice = convertToURL(sAppExe)
  
    '/// Get the TestTool port value from the TestTool control file  
    sTTPort = GetIniValue (gTesttoolIni, "Communication", "TTPort")
    '/// Get the UNO port value from the TestTool control file  
    sResultUno = GetIniValue (gTesttoolIni, "Communication", "UnoPort")
    ' make sure both ports are different
    if sTTPort = sResultUno then
        warnlog "TestTool and UNO port are the same ("+sResultUno+")! Please change the UNO port in the TestTool application: Extra -> Settings -> Misc -> Remote UNO Port and exit OpenOffice.org."
        exit function
    end if
    if NOT bSilent then
        printlog "Trying to use Office/Testtool UNO Port '" + sResultUno + "'."
    endif
    if (sResultUno <> "") then
        sUnoPort = sResultUno
    else
        warnlog ("Please add an entry to your '" + gTesttoolIni + "' in section 'Communication': 'UnoPort=82352' and restart your testtool and exit OpenOffice.org.")
        warnlog ("You also can check the setting in TestTool: Extra->Settings->Misc: and change the value for 'Remote UNO Port' and then exit OpenOffice.org.")
        exit function
    end if
    if (NOT bJustGettingPort) then
        '/// <i>-accept=socket,host=localhost,port=(PortNr);urp</i> has to be added to the start command. 
        sOfficeParameters = "-accept=socket,host=localhost,port=" + sUnoPort + ";urp"
        try
            '/// If this service has been used before the connection will be established.
            hGetUNOService = getUnoApp
            if NOT bSilent then
                printlog "Office/Testtool UNO: CONNECTION SUCCESSFULL"
            end if
            ' If this tree will be used the connection has been established before!
        catch        
            ' If the connection has not been established before this tree will be used.
            '/// If the UNO service has not been used before the application will be <i>started</i> with the additional parameters.
            qaerrorLog "/qa/qatesttool/global/tools/inc/t_tools2.inc::hGetUNOService 'getUnoApp' needn't fail anymore!"
            Shell (sUnoOffice, 1,sOfficeParameters,false)
            if NOT bSilent then
                printlog "Office/Testtool UNO: TRYING TO CONNECT"
            end if
            sleep(10)
        endcatch
        
        '/// This will be tried twice.
        ' Second chance
        if isNull(hGetUNOService) then
            try
                hGetUNOService = GetUnoApp
                if NOT bSilent then
                    printlog "Office/Testtool UNO: CONNECTION SUCCESSFULL"
                endif
            catch
                '/// If the UNO service could not be started a warnlog will be written to the result file. 
                warnlog "Office/Testtool UNO: CONNECTION FAILED"
            endcatch
        end if
    else
        sUnoPortExternal = sUnoPort
    end if
end function

'-------------------------------------------------------------------------

function fopenConfig( sPackage   as String  ,_
                     sPath      as String  ,_
                     bReadWrite as Boolean ,_
                     bAllLocale as Boolean ) as Object
'/// Open a configuration package from the Office installation via UNO API.
'/// <ul><b>Input</b> 
'///+ <li>Parameter: <i>sPackage</i>
'///+ describe the package which should be handled by the returned
'///+ configuration access object
'///+ <u>Example</u>: "/org.openoffice.Office.TypeDetection"</li>
'///+ <li>Parameter: <i>sPath</i>
'///+ Specify the relativ path inside the new opened package,
'///+ where we are interested on
'///+ <u>Example</u>: "Types/xxx" => "/org.openoffice.Office.TypeDetection/Types/xxx"</li>
'///+ <li>Parameter: <i>bReadWrite</i>
'///+ Describe how the package should be opened (readonly/writable)</li>
'///+ <li>Parameter: <i>bAsLocale</i>
'///+ Enable/disable the special ALL LOCALE mode of the configuration API.
'///+ It makes it possible to have access on localized nodes directly instead
'///+ of using the generic handling of used API for it.</li></ul>
'///+ <b>Return</b>: <i>Object</i>
'///+ Object provides access to the required package or directly to a config key.
    Dim sFullPath as String
    Dim aConfig as Object
    Dim aConfigProvider as Object
    Dim lNormalParams(0) as new com.sun.star.beans.PropertyValue
    Dim lLocaleParams(1) as new com.sun.star.beans.PropertyValue
    Dim lParams() as Object
    Dim oUno as Object

    sFullPath = sPackage+"/"+sPath

    if (bAllLocale=true) then
        lLocaleParams(0).Name  = "nodepath"
        lLocaleParams(0).Value = sFullPath
        lLocaleParams(1).Name  = "locale"
        lLocaleParams(1).Value = "*"
        lParams() = lLocaleParams()
    else
        lNormalParams(0).Name  = "nodepath"
        lNormalParams(0).Value = sFullPath
        lParams() = lNormalParams()
    end if

    oUno = hGetUnoService
    
    aConfigProvider = oUno.createInstance("com.sun.star.configuration.ConfigurationProvider")

    if (bReadWrite=true) then
        aConfig = aConfigProvider.createInstanceWithArguments( _
            "com.sun.star.configuration.ConfigurationUpdateAccess", _
            lParams() )
    else
        aConfig = aConfigProvider.createInstanceWithArguments( _
            "com.sun.star.configuration.ConfigurationAccess", _
            lParams() )
    end if

    fopenConfig = aConfig
end function

'-------------------------------------------------------------------------

function fGetProductName as string
'/// Reads the ProductKey from bootstrap/version file and cuts of version number,
    Dim sProduct as string
    Dim sSplit() as string
    Dim i as integer
    Dim u as integer
    Dim sFile as string
    Dim sIniEntry as string
    Dim cFileExt as string
       
    'Using the bootstraprc/bootstrap.ini file in ../program dir
    'to get the value of 'ProductKey'
    
    if ( lcase( gPlatform ) = "osx" ) then
    	sfile = convertPath(gNetzOfficePath + "MacOS/bootstrap")
    else
    	sfile = convertPath(gNetzOfficePath + "program/bootstrap")
    end if
    
    sIniEntry = "Bootstrap"
    
    'Setting the differnt extension to the files.
    if gPlatGroup = "unx" then
        cFileExt = "rc"
    else
        cFileExt = ".ini"
    end if

    'Getting the value of 'ProductKey'-entry or setting it to 'OpenOffice.org 2.0'
    if (dir(sFile+cFileExt) <> "") then
        sProduct = getIniValue(sFile+cFileExt, sIniEntry , "ProductKey")
    else
        warnlog "Could not get the ProductKey value! Setting it to 'OpenOffice.org 2.0' and trying to run the tests!"
        sProduct = "OpenOffice.org 2.0"
    end if
    if (sProduct <> "" AND sProduct <> "NOT EXISTING") then
        sSplit = split(sProduct, " ") ' get count of spaces
        sProduct = ""
        'Presupposition: Version number is not seperated by spaces, 
        'but seperated with space from ProductName
        u = uBound(sSplit)
        if (u > 0) then
            for i = 0 to (u-1)
                sProduct = sProduct + sSplit(i)        ' add strings until last Space
                if (i <> (u-1)) then 
                    sProduct = sProduct + " "
                end if
            next i
        else
            sProduct = sSplit(0)
        end if
    end if
    fGetProductName = sProduct 
end function

'-------------------------------------------------------------------------

function FindBuildID as String
'/// Get BuildID out of <i>bootstrap.ini/boostraprc</i>
'///+or search in <i>.../program/resource/isoxxx??.res</i> for the BuildID.
  Dim sOfficePath as String
  Dim FileNum, iStart, i as Integer
  Dim xmlZeile, sZ1, sZ2, sIsofile as String
  Dim sTemp as String
  Dim sFile as string
  Dim sPlatformProgramPath as string
    
  if (gNetzInst = TRUE) then
     sOfficePath = gNetzOfficePath
   else
     sOfficePath = gOfficePath
  end if

   ' bootstrap.ini/rc part
   if (gSamePC = TRUE) then
        ' since CWS nativefixer18 the information from bootstrap file is spread across bootstrap and version
        if ( lcase( gPlatform ) = "osx" ) then
            sPlatformProgramPath = "MacOS"
        else
            sPlatformProgramPath = "program"
        end if        
        sfile = convertPath(gOfficeBasisPath & "program/version")
        if gPlatGroup = "unx" then
            sFile = sFile + "rc"
            if (dir(sFile) <> "") then
                sTemp = getIniValue(sFile, "Version", "buildid")
                gMajor = getIniValue(sFile, "Version", "ProductSource")
            else
                sfile = convertPath(gNetzOfficePath & sPlatformProgramPath & "/versionrc")
                sTemp = getIniValue(sFile, "Version", "buildid")
            end if
        else
            sFile = sFile + ".ini"
            if (dir(sFile) <> "") then
                sTemp = getIniValue(sFile, "Version", "buildid")
                gMajor = getIniValue(sFile, "Version", "ProductSource")
            else
                sfile = convertPath(gNetzOfficePath & sPlatformProgramPath & "/version.ini")
                sTemp = getIniValue(sFile, "Version", "buildid")
            end if
        end if
   end if
   
   ' fallback to get the buildID via isoxxx??.res part
   if (sTemp = "") then
      sIsofile = Dir (sOfficePath & sPlatformProgramPath & gPathSigne & "resource" & gPathSigne & "iso*.res")
      if sIsofile = "" then
         sIsofile = App.Dir (sOfficePath & sPlatformProgramPath & gPathSigne & "resource" & gPathSigne & "iso*.res")
      end if
      sIsofile = ConvertPath (sOfficePath & sPlatformProgramPath & gPathSigne & "resource" & gPathSigne & sIsofile)
      if sIsofile= "" then
         warnlog "FindBuildID : No isoxxx??.res-file was found!"
         exit function
      end if

      FileNum = FreeFile
      Open sIsofile For Input As #FileNum
      do until EOF(#FileNum) = True
         line input #FileNum, xmlZeile
         for i=1 to 100
             if i=1 then
                sZ1 = left (xmlzeile, 2048)
             else
                sZ1 = left (sZ2, 2048)
             end if
             if sZ1 < 2048 then
                i=101
             else
                iStart = instr (1, sZ1, "Build", 1)
                if iStart <> 0 then
                   iStart = iStart-5
                   sTemp = Mid (sZ1, iStart, 16)
                   exit do
                end if
                sZ2 = right (sZ1, len (sZ1)-2048)
            end if
         next i
      loop
      Close #FileNum
   end if

   ' WorkAround version information starting with 'SRC' or any other letter code as announced
   iStart = len(sTemp)
   i = 1
   ' take the first character
   sZ1 = mid(sTemp,i,1)
   ' if there is more than one character in the string AND the first character is not a number
   if ((iStart > 0) AND (NOT isNumeric(sZ1))) then
       ' increment counter as long as there is no number found in the string
       while ((i < iStart) AND (NOT isNumeric(mid(sTemp,i,1)) ))
           inc(i)
       wend
       ' cut of the not number characters at the start of the string
       sTemp = right(sTemp, len(sTemp)-(i-1))
   end if
   
   FindBuildID = sTemp
end function

'-------------------------------------------------------------------------

sub hSetBuildVersionInformation(bQuite as boolean)
'/// set global version information variables: gMajor, gMinor, gBuild ///'
'/// presupposition: global variable gVersionsnummer is initialised by FindBuildID() ///'
    dim slVersion() as string
    dim ilVersion as integer
    dim sLastVersion as string
    dim iPosA as integer
    dim iPosB as integer

    slVersion() = Split(gVersionsnummer, ",") 
    ilVersion = uBound(slVersion()) ' array counts from 0 on!
    sLastVersion = slVersion(ilVersion)
    ' major is from start to 'm'
    iPosA = 1
    iPosB = instr(sLastVersion, "m")
    if (iPosB = 0) then ' there is no minor
        if (Not bQuite) then
            warnlog "Product Version Information is missing (mXX). Please tell the developer to build with 'setsolar -ver'"
        endif
        iPosB = instr(sLastVersion, "(")
    endif
    if gMajor = "" then
        gMajor  = Mid(sLastVersion, iPosA, (iPosB-iPosA))  '(1) Major
    endif
    iPosA = iPosB
    iPosB = instr(sLastVersion, "(")
    gMinor  = Mid(sLastVersion, iPosA, iPosB-iPosA)        '(2) Minor
    iPosA = instr(sLastVersion, ":") + 1
    iPosB = instr(sLastVersion, ")")
    gBuild = cInt(Mid(sLastVersion, iPosA, iPosB-iPosA))   '(3) Build
end sub

'-------------------------------------------------------------------------

function fRelativeToAbsolutePath (sRelativePath as string) as string
'/// INPUT: provide a path with relative indicators ".." ///'
'///+ The input needs to konsist of the parts: where was the relative string found, and ///'
'///+ the relative path itself as one string. E.g: "/opt/var/../../here/is/it"///'
'/// RETURN: String with the removed parts for each relative iteration. E.g. This returns: "/here/is/it"///'

    dim iHowOften as string
    dim aSplitOnDoublePoints() as string
    dim aSplitOnPathSign() as string
    dim aJoinWithPathSign() as string
    dim i,x,y as integer
    dim sIntern as string

    ' save the input
    sIntern = sRelativePath
    ' get count of 'relative path ups'
    aSplitOnDoublePoints = split(sIntern, gPathSigne+"..")
    ' for every occurrence cut part from path
    iHowOften = uBound(aSplitOnDoublePoints)-1
    for i = 0 to iHowOften
        ' Split on every "/.."
        aSplitOnDoublePoints = split(sIntern, gPathSigne+"..")
        ' always work on the first part (The one before the first "/..")
        ' Split the first path at the PathSeperators
        aSplitOnPathSign = split(aSplitOnDoublePoints(0), gPathSigne)
        ' define new size for the first part destination
        redim aJoinWithPathSign(uBound(aSplitOnPathSign())-1)
        ' copy the parts, but not the last part
        for x = 0 to uBound(aJoinWithPathSign())
            aJoinWithPathSign(x) = aSplitOnPathSign(x)
        next x
        ' make one string of the parts with PathSeperators
        aSplitOnDoublePoints(0) = join(aJoinWithPathSign(), gPathSigne)
        ' cut the .. for this run from the string
        redim aJoinWithPathSign(uBound(aSplitOnDoublePoints())-1)
        y=0
        for x = 0 to uBound(aJoinWithPathSign())+1
            if x <> 1 then
                aJoinWithPathSign(x-y) = aSplitOnDoublePoints(x)
            else
                y=1
            endif
        next x
        ' set put all parts together again into one string
        if iHowOften <> i then
            sIntern = join(aJoinWithPathSign(), gPathSigne+"..")
        else
            sIntern = join(aSplitOnDoublePoints(), "")
        endif
    next i
    ' set the returnvalue
    fRelativeToAbsolutePath = sIntern
end function

'-------------------------------------------------------------------------

sub sCheckValgrindStatus()
    ' valgrind only exists on Linux 
    ' If testlauncher is started with parameter --valgrind, a file called
    ' $HOME/tcs.txt is created, with the name of the .bas file
    ' If you don't know the testlauncher, just make sure that the file is created
    ' and contains the name of the .bas file, if you want to use valgrind tests.
    Dim sTestCaseSpecification as string
    Dim sList(10) as string
    Dim sTemp as string
    
    sTemp = environ("HOME")
    sTemp = sTemp + "/tcs.txt"
    if fileExists(sTemp) then
        ListRead(sList(), sTemp)
        if (ListCount(sList())>0) then
            sTemp = sList(1)
            sTemp = right(sTemp, len(sTemp)-1)
            printlog "**  Valgrind mode detected: '" + sTemp + "'"
            setChildEnv("tcs",sTemp)
        end if
    end if
end sub

'-------------------------------------------------------------------------

function fgetDocumentLanguages(byRef aDefaultLocale(), optional bInteger as boolean)
'/// INPUT: aDefaultLocale - array from 0 to 3
'/// INPUT: OPTIONAL: bInteger - TRUE: return language as number en: 1; FALSE: (default) return the short text for locale e.g en_US
'/// RETURN: write in the deliverd array aDefaultLocale depending on bInteger the language from Tools->Options->Language Settings->Languages->Default language for documents
'///+ either the short string representing the language (default) e.g. en_US or the number e.g. 1
'///+ The index of the array is defined:
'///+ (1) Western
'///+ (2) Asian
'///+ (3) CTL

    dim uno
    dim ap
    dim xViewRoot
    dim apara(1) As new com.sun.star.beans.PropertyValue
    dim i as integer
    dim blInteger as boolean

    if isMissing(bInteger) then
        blInteger = FALSE
    else
        blInteger = bInteger
    endif

    uno=hGetUnoService(true)
    ap=uno.createInstance("com.sun.star.configuration.ConfigurationProvider")
    apara(0).Name="nodepath"
    apara(0).Value="/org.openoffice.Office.Linguistic/General"
    apara(1).Name="lazywrite"
    apara(1).Value=False
    xViewRoot=ap.createInstanceWithArguments("com.sun.star.configuration.ConfigurationUpdateAccess",aPara())
    aDefaultLocale(1) = xViewRoot.getPropertyValue("DefaultLocale")
    aDefaultLocale(2) = xViewRoot.getPropertyValue("DefaultLocale_CJK")
    aDefaultLocale(3) = xViewRoot.getPropertyValue("DefaultLocale_CTL")
    xViewRoot.dispose()
    
    ' If the return of the language number is requested, convert it
    if blInteger then
        for i = 1 to 3
            if aDefaultLocale(i) <> "" then
                aDefaultLocale(i) = convertLanguage2(aDefaultLocale(i))
            else
                aDefaultLocale(i) = 0
            endif
        next i
    endif
end function

'-------------------------------------------------------------------------

function hDisableQuickstarterAPI as boolean
    Dim xQuickStarter as object
    Dim oUnoOfficeConnection as object
    Dim bResult as boolean

    bResult = TRUE
    'Second, closing the Quickstarter process that a restart of the office
    'would result into one process (the Quickstart would hinder otherwise
    'the communication to the office.    
    'On mac this results in a disbaled quickstarter imediately, but not persistant on restart.
    oUnoOfficeConnection=hGetUnoService(TRUE)
    if (isNull(oUnoOfficeConnection)) then
        QAErrorLog "Couldn't create UNO access. Can't disable Quickstarter via UNO API."                
        bResult = FALSE
    else
        try
            xQuickStarter = oUnoOfficeConnection.createInstance("com.sun.star.office.Quickstart")        
            'DEBUG: printlog xQuickStarter.dbg_supportedinterfaces
            'disable quickstart veto (not quickstart UI)
            xQuickStarter.setFastPropertyValue(0, FALSE)        
        catch
            qaErrorLog "Join Quickstarter and OOo process failed. There will be problems on shutdown"
            bResult = FALSE
        endcatch
    end if
    hDisableQuickstarterAPI = bResult    
end function

'-------------------------------------------------------------------------

