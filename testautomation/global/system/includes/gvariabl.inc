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
'* short description : all global variables
'*
'************************************************************************
'*
' #1 OnlyForDocuGVariables
'*
'\***********************************************************************

'/// * System for the installed Office
Global gPlatform as String          '///+ gPlatform => Operatingsystem (internal shortkey)
Global gSYSName as String           '///+ gSYSName => Name of Operatingsystems
Global gSystemPath as String        '///+ gSystemPath => Systempath
Global gPathSigne as String         '///+ gPathSigne => Pathsigne for the Operationsystem
Global gPlatGroup as String         '///+ gPlatGroup => the systemgroup  for the OS  (e.g. 'unx' 'w95' etc.)
Global iSystemSprache as Integer    '///+ iSystemSprache => code for the systemlanguage (01 = en_us, 40 = de_de)
Global gSVersionIni as String       '///+ gSVersionIni => path to sversion.ini

'/// * System for the Testtool
Global gtSYSName as String          '///+ gtSYSName => Name of Testtool-Operationsystem
Global gtPlatform as String         '///+ gtPlatform => Operatingsystem (internal shortkey)
Global gtSystemPath as String       '///+ gtSystemPath => Users Home directory
Global gtCommonSystemPath as String '///+ gtCommonSystemPath => Global directory AllUser or /etc
Global gTesttoolIni as String       '///+ gTesttoolIni => Path to Testtool.ini
Global gtHidLstPath as String       '///+ gtHidLstPath => Path to hid.lst
Global gTTProfileName as string     '///+ gTTProfileName => The current used Profile

'/// * Settings for Crashreporter
Global gUseProxy                    '///+ Value of [Crashreporter] 'UseProxy' from the testtool control file: Is TRUE or FALSE
Global gProxyServer                 '///+ Value of [Crashreporter] 'ProxyServer' from the testtool control file: string
Global gProxyPort                   '///+ Value of [Crashreporter] 'ProxyPort' from the testtool control file: string
Global gReturnAddress               '///+ Value of [Crashreporter] 'ReturnAddress' from the testtool control file: string
Global gAllowContact                '///+ Value of [Crashreporter] 'AllowContact' from the testtool control file: Is TRUE or FALSE
Global gConnectionType              '///+ Value is depending on gUseProxy: TRUE = MANUALPROXY, FALSE = DIRECT

'/// * path - variables
Global gOfficePath as String        '///+ gOfficePath => included the path of the Office-installation
Global gOfficeBasisPath as String   '///+ gOfficeBasisPath => included the path of the Office-basis-installation
Global gRemotePath as String        '///+ gRemotePath => Base Directory on Remote system
Global gTesttoolPath as String      '///+ gTesttoolPath => included the path of the Testtool-directory (changed with ConvertPath)
Global oTesttoolPath as String      '///+ oTesttoolPath => original Testtool-directory
Global sAppExe as String            '///+ sAppExe => included the whole path to 'soffice.exe' or 'soffice'
Global sFactory as String           '///+ sFactory => Which application module should be used swriter, scalc, ...
Global sAppParameter as String      '///+ sAppParameter => included all parameters for the executable
Global sCapturePath as String       ' path for graphic-snapshots
Global gPrivateEnvironmentLocation as string '///+ gPrivateEnvironmentLocation: Location for test environment special data and files

'/// * Officevariables
Global iSprache as Integer          '///+ iSprache  => code for the office-language (01=en_us, 49=de_de etc.)
Global bAsianLan as Boolean         '///+ bAsianLan => if the office-language is an asian language (81, 82, 86, 88) => TRUE
Global bCTLLan as Boolean           '///+ bCTLLan   => if the office-language is an CTL language (66, 91, 96, 97) => TRUE
Global gLanguage as String          '///+ gLanguage => the languagename for office language
Global gISOLang as String           '///+ gISOLang  => the language code ISO 639 (and ISO 3166 for en language)
Global gAsianSup as Boolean         '///+ gAsianSup => if asian support is activated in options
Global gCTLSup as Boolean           '///+ gCTLSup   => if CTL support is activated in options
Global gAccessibility as Boolean    '///+ gAccessibility => If an accessibility tool is detected (gnome, JAWS)
Global gProductName as string        '///+ gProductName => Name of the product (StarOffice, OpenOffice.org, ...)
Global gMacroSecurityLevel as integer'///+ gMacroSecurityLevel => Macro Security Level (0: Low ... 3: Very High) 
Global gOfficeVersion as string     '///+ gOfficeVersion => The version number of the office like 3.1, 9.0 ...
Global gAutoUpdate as boolean     '///+ gAutoUpdate => if office is able to request update streams

'/// - global OLE-objectnames
global gOLEWriter as String         '///+ gOLEWriter       => OLE-object-name for Writer
global gOLECalc as String           '///+ gOLECalc         => OLE-object-name for Calc
global gOLEImpress as String        '///+ gOLEImpress      => OLE-object-name for Impress
global gOLEDraw as String           '///+ gOLEDraw         => OLE-object-name for Draw
global gOLEChart as String          '///+ gOLEChart        => OLE-object-name for Chart
global gOLEMath as String           '///+ gOLEMath         => OLE-object-name for Math
global gOLEOthers as String         '///+ gOLEOthers       => OLE-object-name for the others-entry

'/// * variables, for the applications
Global gApplication as String       '///+ gApplication => included the tested application (DRAW, WRITER etc.)
Global gNoNewDoc as Boolean         ' old

'/// * other variables
Global gClient as Boolean           '///+ gClient => TRUE : Client will be tested; FALSE : files will be saved/loaded on clientside
Global gClientLaden as Boolean      '///+ gClientLaden => TRUE = files will be saved/loaded on serverside (only for UNIX-systems) ; FALSE : files will be saved/loaded on clientside
Global gClientUser as String        '///+ gClientUser => Username of Client-User
Global gClientUserPWD as String     '///+ gClientUserPWD =>  password of Client-User
Global gsClient as String           '///+ gsClient =>  path to the sclient.exe
Global gClientAufUNIX as String     ' old
Global sServerIni as String         ' old

Global gNetzInst as Boolean         '///+ gNetz => TRUE : Netinstallation will be tested; FALSE : normal full-version will be tested
Global gNetzOfficePath as String    '///+ gNetzOfficePath => path to the Network-Installation

Global gSamePC as Boolean           '///+ gSamePC => TRUE : StarOffice and Testtool run on same mashine
Global gPort as String              ' Portnumber for communication between Testtool and Office
Global gHost as String              '///+ gHost => for remote-test : Hostname for Office-OS
Global gPCName as String            '///+ gPCname => name for the mashine, where Testtool runs

'/// *
Global gDasNicht                    '///+ gDasNicht => 0 the translation-mechanism will be activated
Global gbSnapShot as Boolean        '///+ gbSnapShot => true Dialogtest-routines create screenshots
Global gVersionsnummer as String    '///+ gVersionsnummer => full version number (major + minor + buildID)
Global gMajor as string             '///+ gMajor: only something like 'src680'
Global gMinor as string             '///+ gMinor: only something like 'm37'
Global gBuild as integer            '///+ gBuild: only something like 4711
Global gUser as String              '///+ gUser => User-LoginName (e.g. 'tz')
Global gInputbox as Boolean         ' old
Global sSchedule as String          ' old
Global bDebugVersion as Boolean     '///+ bDebugVersion => TRUE : Debugversion; FALSE : normal Producktversion of Office
Global gCWS as Boolean              '///+ gCWS = TRUE if a Child WorkSpace (CWS) has been detected.
Global gStatusDatabase as Boolean   '/// gStatusDatabase = TRUE if the testscripts are running on a TT-Framework-server.  
Global gLocalStatusDatabase as string '///  <> "" set to a local path string, to work with a local status database
Global gStatusFeatureLevel as Integer '///+ gStatusFeatureLevel : Controled from privateenvironment.inc
Global gUseSysDlg                   '///+ gUseSysDlg => FALSE : use internal file-dialogs of StarOffice; => TRUE : use system-file-dialogs
Global gOOO as boolean              '///+ gOOO => if an OpenOffice.org-version is tested, this variable is TRUE
Global gOOoShutdownTimeOut as integer'///+ gOOoShutdownTimeOut => max. time to wait in seconds, after quiting OOo
Global gOOoStartupTimeOut as integer'///+ gOOoStartupTimeOut => max. time to wait in seconds, for starting OOo
Global gStartTheOffice as boolean '///+ gStartTheOffice => indicates an alternative startroutine is run

'///* variables only for creating the status-pages
Global gTestDate as String          '///+ gTestDate : fix date of startup
Global gTestTime as String          '///+ gTestTime : fix time of startup
Global gTestName as String          '///+ gTestName : name of the test (must be set as parameter in hStatusIn())
Global gTestAppArea as String       '///+ gTestAppArea : name of the testarea (e.g. writer, calc etc.) - (must be set as parameter in hStatusIn())
Global gErrorSum as Integer         '///+ gErrorSum : last value of errors in a test
Global gWarningSum as Integer       '///+ gWarningSum : last value of warnings in a test
Global gQaErrorSum as Integer       '///+ gQaErrorSum : last value of qaErrors in a test
Global gStatusDuration              '///+ gStatusDuration : starting time to get at the end the duration of a test
Global gTestcaseStart as Date       '///+ gTestCaseDuration : starting time of the testcase
Global glsStatusPage (32000) as String  '///+ glsStatusPage () : list for the output in the status-page
Global gTestDescr as String         '///+ short descrition for the test
Global gTestFileName as String      ' only a dummy variable, until GH give me a new method in testtool
Global gDatabasePath as string      '///+ top directory, of location for wget and result database files

'///* Private environment values; values get set in qa/qatesttool/global/private/inc/privateenvironment.inc
Global privateDatabaseServerName as string
Global privateDatabaseServerIP as string
Global privateProxyServer as string
Global privateProxyPort as integer
Global privateDatabasePath as string
Global privateDatabaseServerPath as string

'///* Macro Security Settings - Symbolic names for security levels (constants) 
Global Const GC_MACRO_SECURITY_LEVEL_LOW      = 0  '///+ GC_MACRO_SECURITY_LEVEL_LOW:      Symbolic name for low macro security
Global Const GC_MACRO_SECURITY_LEVEL_MEDIUM   = 1  '///+ GC_MACRO_SECURITY_LEVEL_MEDIUM:   Symbolic name for medium macro security
Global Const GC_MACRO_SECURITY_LEVEL_HIGH     = 2  '///+ GC_MACRO_SECURITY_LEVEL_HIGH:     Symbolic name for high macro security
Global Const GC_MACRO_SECURITY_LEVEL_VERYHIGH = 3  '///+ GC_MACRO_SECURITY_LEVEL_VERYHIGH: Symbolic name for very high macro security
Global Const GC_MACRO_SECURITY_LEVEL_DEFAULT  = 2  '///+ GC_MACRO_SECURITY_LEVEL_DEFAULT:  Symbolic name for default macro security level

'///* OOo Improvement Program
Global gOOoImprovementIsEnabled as boolean

'///* Performance flags, set to true to eliminate wait/sleep statements, default is FALSE
Global global_use_new_sleep as boolean

'///* If set to TRUE  many functions become more talkative, default is FALSE
Global gVerbose as boolean

'///* When using hFileOpenLocally(...) we do not know the filename under which the file has been saved.
Global gLastWorkFile as string

sub OnlyForDocuGVariables
'///<b>System for the installed Office</b>
'///<i>gPlatform</i>: Operatingsystem (internal shortkey)
'///<i>gSYSName</i>: Name of Operatingsystems
'///<i>gSystemPath</i>: Systempath
'///<i>gPathSigne</i>: Pathsigne for the Operationsystem
'///<i>gPlatGroup</i>: The systemgroup  for the OS  (e.g. 'unx' 'w95' etc.)
'///<i>iSystemSprache</i>: Code for the systemlanguage (01 = en_us, 40 = de_de)
'///<i>gSVersionIni</i>: Path to sversion.ini

'///<b>System for the Testtool</b>
'///<i>gtSYSName</i>: Name of Testtool-Operationsystem
'///<i>gtPlatform</i>: Operatingsystem (internal shortkey)
'///<i>gtSystemPath</i>: Systempfad
'///<i>gTesttoolIni</i>: Path to Testtool.ini
'///<i>gtHidLstPath</i>: Path to hid.lst

'///<b>Settings for Crashreporter</b>
'///<i>gUseProxy</i>: Value of [Crashreporter] 'UseProxy' from the testtool control file: Is TRUE or FALSE
'///<i>gProxyServer</i>: Value of [Crashreporter] 'ProxyServer' from the testtool control file: string
'///<i>gProxyPort</i>: Value of [Crashreporter] 'ProxyPort' from the testtool control file: string
'///<i>gReturnAddress</i>: Value of [Crashreporter] 'ReturnAddress' from the testtool control file: string
'///<i>gAllowContact</i>: Value of [Crashreporter] 'AllowContact' from the testtool control file: Is TRUE or FALSE
'///<i>gConnectionType</i>: Value is depending on gUseProxy: TRUE = MANUALPROXY, FALSE = DIRECT

'///<b>Path - variables</b>
'///<i>gOfficePath</i>: Includes the path of the Office-installation
'///<i>gTesttoolPath</i>: Includes the path of the Testtool-directory (changed with ConvertPath)
'///<i>oTesttoolPath</i>: Original Testtool-directory
'///<i>sAppExe</i>: Includes the whole path to 'soffice.exe' or 'soffice'
'///<i>sAppParameter</i>: Includes all parameters for the executable
'///<i>sCapturePath</i>: Path for graphic-snapshots
'///<i>gPrivateEnvironmentLocation</i>: Location for test environment special data and files

'///<b>Officevariables</b>
'///<i>iSprache </i>: Code for the office-language (01=en_us, 49=de_de etc.)
'///<i>bAsianLan</i>: If the office-language is an asian language (81, 82, 86, 88)</i>: TRUE
'///<i>gLanguage</i>: The languagename for office language
'///<i>gMacroSecurityLevel</i>: Macro Security Level (0: Low ... 3: Very High) 

'///<b>global OLE-objectnames</b>
'///<i>gOLEWriter</i>: OLE-object-name for Writer
'///<i>gOLECalc</i>: OLE-object-name for Calc
'///<i>gOLEImpress</i>: OLE-object-name for Impress
'///<i>gOLEDraw</i>: OLE-object-name for Draw
'///<i>gOLEChart</i>: OLE-object-name for Chart
'///<i>gOLEMath</i>: OLE-object-name for Math
'///<i>gOLEOthers</i>: OLE-object-name for the others-entry

'///<b>variables, for the applications</b>
'///<i>gApplication</i>: Includes the tested application (DRAW, WRITER etc.)

'///<b>other variables</b>
'///<i>gClient</i>:<ul><li>TRUE: Client will be tested</li><li>FALSE: Files will be saved/loaded on client side</li></ul>
'///<i>gClientLaden</i>:<ul><li>TRUE: Files will be saved/loaded on server side (only for UNIX-systems)</li><li>FALSE: Files will be saved/loaded on client side</li></ul>
'///<i>gClientUser</i>: Username of Client-User
'///<i>gClientUserPWD</i>:  Password of Client-User
'///<i>gsClient</i>:  Path to the sclient.exe
'///<i>gNetz</i>:<ul><li>TRUE: Net installation will be tested</li><li>FALSE: Normal full/FAT-version will be tested</li></ul>
'///<i>gNetzOfficePath</i>: Path to the Network-Installation
'///<i>gSamePC</i>:<ul><li>TRUE: Office and Testtool run on same mashine</li></ul>
'///<i>gHost</i>: (For remote-tests) Hostname for Office OE/OS
'///<i>gPCname</i>: Name for the machine, where the Testtool runs

'///<b>Variables only for creating the status-pages</b>
'///<i>gTestDate</i>: Fix date of startup
'///<i>gTestTime</i>: Fix time of startup
'///<i>gTestName</i>: Name of the test (must be set as parameter in <i>hStatusIn()</i>)
'///<i>gTestAppArea</i>: Name of the office module (e.g. writer, calc etc.) - (must be set as parameter in <i>hStatusIn()</i>)
'///<i>gErrorSum</i>: Last value of errors in a test
'///<i>gWarningSum</i>: Last value of warnings in a test
'///<i>gStatusDuration</i>: Starting time to get at the end the duration of a test
'///<i>glsStatusPage()</i>: List for the output in the status-page
'///<i>gStatusFeatureLevel</i> : Controled from privateenvironment.txt

'///<b>Specials</b>
'///<i>gDasNicht</i>: 0 the translation-mechanism will be activated
'///<i>gbSnapShot as Boolean</i>:<ul><li>TRUE: Dialogtest-routines create screenshots</li></ul>
'///<i>gVersionsnummer</i>: Full version number (major + minor + buildID)
'///<i>gUser</i>: User login name (e.g. 'tz')
'///<i>bDebugVersion</i>:<ul><li>TRUE: Debugversion</li><li>FALSE: normal Producktversion of office</li></ul>
'///<i>gUseSysDlg</i>:<ul><li>FALSE: Use internal file-dialogs of the office</li><li>TRUE: Use system-file-dialogs</li></ul>
'///<i>gOOO</i>: If an OpenOffice.org-version has been detected, this variable is TRUE
'///<i>gOOoShutdownTimeOut</i>: Max. time to wait in seconds, after quiting OOo
'///<i>gOOoStartupTimeOut</i>: Max. time to wait in seconds, for starting OOo
end sub

