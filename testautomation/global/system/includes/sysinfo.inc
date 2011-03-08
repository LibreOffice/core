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
'* short description : Establish facts of the operating system
'*
'************************************************************************
'*
' #1 PlatFormInit         ' Insert the list variables PlatformName, PlatformExt and PlatormGroup
' #1 getToolPlatform      ' Affected variables: gtSystemPath, gTesttoolIni, gtPlatform, gSamePC, gtSYSName, gUser and gHost.
' #1 GetOfficePlatform    ' Affected variables: gSamePC, gSystemPath, gSYSName, gTesttoolPath and gOfficePath. 
' #1 hGetSystemLanguage   ' Get the system language
' #0 hfGetTTProfileName
'*
'\***********************************************************************

' define the global definitions for operating systems
'
Global Const MAX_PLATFORM = 25
Global gSystemnummer as Integer

' Systeme und Versionen
Global Const SYS_00    = 0
Global Const SYS_W95   = 1
Global Const SYS_W98   = 2
Global Const SYS_WSE   = 3
Global Const SYS_WME   = 4
Global Const SYS_WNT   = 5
Global Const SYS_W2K   = 6
Global Const SYS_WXP   = 7
Global Const SYS_MAC   = 8
Global Const SYS_SOL   = 9
Global Const SYS_LIN   = 10
Global Const SYS_x86   = 11
Global Const SYS_OSX   = 12
Global Const SYS_PPC   = 13
Global Const SYS_AIX   = 15
Global Const SYS_HP    = 17
Global Const SYS_FBSD  = 18
Global Const SYS_NBSD  = 19
Global Const SYS_LIN64 = 20
Global Const SYS_LINSPARC = 21
Global Const SYS_ECS   = 22

Global PlatFormGroup(MAX_PLATFORM) as String
Global PlatFormName(MAX_PLATFORM) as String
Global PlatFormExt(MAX_PLATFORM) as String

sub PlatFormInit
'///<b>Initialize information arrays</b>
'///+Supported Operating Systems:
'///+<TABLE BORDER=1><TR><TH>Name</TH><TH>Short Name</TH><TH>Platgroup</TH></TR>
'///+<TR><TD>Win95</TD><TD>w95</TD><TD>w95</TD></TR>
'///+<TR><TD>Win98</TD><TD>w98</TD><TD>w95</TD></TR>
'///+<TR><TD>Win98SE</TD><TD>wse</TD><TD>w95</TD></TR>
'///+<TR><TD>Win98ME</TD><TD>wme</TD><TD>s95</TD></TR>
'///+<TR><TD>WinNT</TD><TD>wnt</TD><TD>w95</TD></TR>
'///+<TR><TD>Win2000</TD><TD>w2k</TD><TD>w95</TD></TR>
'///+<TR><TD>WinXP</TD><TD>wxp</TD><TD>w95</TD></TR>
'///+<TR><TD>Solaris SPARC</TD><TD>sol</TD><TD>unx</TD></TR>
'///+<TR><TD>Solaris x86</TD><TD>x86</TD><TD>unx</TD></TR>
'///+<TR><TD>Linux</TD><TD>lin</TD><TD>unx</TD></TR>
'///+<TR><TD>Mac OS X</TD><TD>osx</TD><TD>unx</TD></TR>
'///+<TR><TD>Linux PPC</TD><TD>ppc</TD><TD>unx</TD></TR>
'///+<TR><TD>AIX</TD><TD>aix</TD><TD>unx</TD></TR>
'///+<TR><TD>HP UNIX</TD><TD>hp</TD><TD>unx</TD></TR>
'///+<TR><TD>FreeBSD</TD><TD>fbsd</TD><TD>unx</TD></TR>
'///+<TR><TD>NetBSD</TD><TD>nbsd</TD><TD>unx</TD></TR>
'///+<TR><TD>Linux 64 bit</TD><TD>lin64</TD><TD>unx</TD></TR>
'///+<TR><TD>Linux SPARC</TD><TD>linsparc</TD><TD>unx</TD></TR>
'///+<TR><TD>eComStation</TD><TD>ecs</TD><TD>ecs</TD></TR>
'///+</TABLE>
   PlatFormName(SYS_00)    = "Unknown"        : PlatFormExt(SYS_00)   = "a00"
   PlatFormName(SYS_W95)   = "Win95"          : PlatFormExt(SYS_W95)  = "w95"
   PlatFormName(SYS_W98)   = "Win98"          : PlatFormExt(SYS_W98)  = "w98"
   PlatFormName(SYS_WSE)   = "Win98SE"        : PlatFormExt(SYS_WSE)  = "wse"
   PlatFormName(SYS_WME)   = "Win98ME"        : PlatFormExt(SYS_WME)  = "wme"
   PlatFormName(SYS_WNT)   = "WinNT"          : PlatFormExt(SYS_WNT)  = "wnt"
   PlatFormName(SYS_W2K)   = "Win2000"        : PlatFormExt(SYS_W2K)  = "w2k"
   PlatFormName(SYS_WXP)   = "WinXP"          : PlatFormExt(SYS_WXP)  = "wxp"
   PlatFormName(SYS_SOL)   = "Solaris SPARC"  : PlatFormExt(SYS_SOL)  = "sol"
   PlatFormName(SYS_X86)   = "Solaris x86"    : PlatFormExt(SYS_X86)  = "x86"
   PlatFormName(SYS_LIN)   = "Linux"          : PlatFormExt(SYS_LIN)  = "lin"
   PlatformName(SYS_OSX)   = "Mac OS X"       : PlatformExt(SYS_OSX)  = "osx"
   PlatformName(SYS_PPC)   = "Linux PPC"      : PlatformExt(SYS_PPC)  = "ppc"
   PlatFormName(SYS_AIX)   = "AIX"            : PlatFormExt(SYS_AIX)  = "aix"
   PlatFormName(SYS_HP)    = "HP UNIX"        : PlatFormExt(SYS_Hp)   = "hp"
   PlatFormName(SYS_FBSD)  = "FreeBSD"        : PlatFormExt(SYS_FBSD) = "fbsd"
   PlatFormName(SYS_NBSD)  = "NetBSD"         : PlatFormExt(SYS_NBSD) = "nbsd"
   PlatFormName(SYS_LIN64) = "Linux 64 bit"   : PlatFormExt(SYS_LIN64)= "lin64"
   PlatFormName(SYS_LINSPARC) = "Linux SPARC" : PlatFormExt(SYS_LINSPARC) = "linsparc"
   PlatFormName(SYS_ECS)      = "eComStation" : PlatFormExt(SYS_ECS)  = "ecs"

   PlatFormGroup(SYS_00)  = "a00"
   PlatFormGroup(SYS_W95)  = "w95"
   PlatFormGroup(SYS_W98)  = "w95"
   PlatFormGroup(SYS_WSE)  = "w95"
   PlatFormGroup(SYS_WME)  = "w95"
   PlatFormGroup(SYS_WNT)  = "w95"
   PlatFormGroup(SYS_W2K)  = "w95"
   PlatFormGroup(SYS_WXP)  = "w95"
   PlatFormGroup(SYS_SOL)  = "unx"
   PlatFormGroup(SYS_X86)  = "unx"
   PlatFormGroup(SYS_LIN)  = "unx"
   PlatFormGroup(SYS_OSX)  = "unx"
   PlatFormGroup(SYS_PPC)  = "unx"
   PlatFormGroup(SYS_AIX)  = "unx"
   PlatFormGroup(SYS_HP)   = "unx"
   PlatFormGroup(SYS_FBSD) = "unx"
   PlatFormGroup(SYS_NBSD) = "unx"
   PlatFormGroup(SYS_LIN64)= "unx"
   PlatFormGroup(SYS_LINSPARC) = "unx"
   PlatFormGroup(SYS_ECS)  = "ecs"
end sub

'-------------------------------------------------------------------------

sub GetToolPlatform
'///<b>Get all information about the system where the testtool runs.</b>
'///The following global variables will be set:
'///+<ul><li><i>gtSystemPath</i>: Systenmpath of the operating system (Testtool)</li>
'///+<li><i>gTesttoolIni</i>: Path to <i>testtool.ini</i></li>
'///+<li><i>gtPlatform</i>: Name of the operating system (3 digit e.g. 'w95')</li>
'///+<li><i>gSamePC</i>: Bolean if the testtool and the office run on same system</li>
'///+<li><i>gtSYSName</i>: Full name of the operating system</li>
'///+<li><i>gUser</i>: Username on the testtool system</li>
'///+<li><i>gHost</i>: The hostname</li></ul>
  Dim GUI as Integer
  Dim iGUIVersion as Integer
  Dim gtIniPath as String
  Dim i% : Dim k%

  Call PlatFormInit
'///Get GUI information -&gt; global definition of the operating system (Windows, UNIX, etc.)
'///+ GUI is a systemvariable, it can get by testtool (1 =&quot; Windows; 4 =&gt; UNIX [<b>depricated</b> 2 was &quot; OS/2;])
   GUI = GetGUIType
   select case GUI
      case 1    : gtSystemPath = GetApplicationPath + "\"            'Change from 'windir' to internal instruction
                  gUser = environ ("vcsid")
                  if (gUser = "") then
                        gUser = environ ("USERNAME")
                  end if
                  gtIniPath = gtSystemPath
      case 2    : ' For eComStation all variables must point to the same directory
                  gUser = environ ("HOME")
                  if ( gUser = "" ) then
                     warnlog( "Please add the HOME variable to the environment and make it point to the directory you wat to store your personal setting in."
                  end if
                  gtSystemPath = gUser & "\"
                  gtIniPath = gtSystemPath
      case 4    : gtSystemPath = getApplicationPath + "/"
                  gUser = environ ("USER")
                  if (gUser = "") then
                        gUser = environ ("LOGNAME")  ' on some Unix systems
                  end if
                  gtIniPath = gtSystemPath
      case else : QAErrorLog "Not supported operating system! Please write an issue about it (qa/testscripts)! The 'GetGUIType' command is returning '" & GUI & "'."
                  QAErrorLog "Setting 'GUI' variable to 4 (=Unix) to try testing. If it fails you have to wait until your issue will have been fixed."
                  GUI = 4
                  gtSystemPath = getApplicationPath + "/"
                  gUser = environ ("USER")
                  if (gUser = "") then
                        gUser = environ ("LOGNAME")  ' on some Unix systems
                  end if
                  gtIniPath = gtSystemPath                  
   end select
'///Get GUI version -&gt; Detailed definition of the operating system (Win95, WinNT, Win2000, Linux, Sparc, x86 etc.)
'///+This data can be get only out of the <i>testtool.ini/.testtoolrc</i>
'///+Here is the list, which entries for which OS
'///+<TABLE BORDER=1><TR><TH>Name</TH><TH>Value</TH></TR>
'///+<TR><TD>Win95</TD><TD>100</TD></TR>
'///+<TR><TD>Win98</TD><TD>395</TD></TR>
'///+<TR><TD>WinNT</TD><TD>351 or 400</TD></TR>
'///+<TR><TD>WinSE</TD><TD>410</TD></TR>
'///+<TR><TD>WinME</TD><TD>490</TD></TR>
'///+<TR><TD>Win2000</TD><TD>500</TD></TR>
'///+<TR><TD>WinXP</TD><TD>501</TD></TR>
'///+<TR><TD>Solaris SPARC</TD><TD>01</TD></TR>
'///+<TR><TD>Linux</TD><TD>03</TD></TR>
'///+<TR><TD>Solaris x86</TD><TD>05</TD></TR>
'///+<TR><TD>Mac OS X</TD><TD>12</TD></TR>
'///+<TR><TD>Linux PPC</TD><TD>13</TD></TR>
'///+<TR><TD>AIX</TD><TD>04</TD></TR>
'///+<TR><TD>HP UNIX</TD><TD>07</TD></TR>
'///+<TR><TD>FreeBSD</TD><TD>08</TD></TR>
'///+<TR><TD>NetBSD</TD><TD>14</TD></TR>
'///+<TR><TD>Linux 64 bit</TD><TD>15</TD></TR>
'///+<TR><TD>Linux SPARC</TD><TD>16</TD></TR>
'///+<TR><TD>eComStation</TD><TD>17</TD></TR>
'///+</TABLE>
'///Get the path to <i>testtool.ini/.testtoolrc</i>
   if GUI = 4 then
      gTesttoolIni = gtIniPath + ".testtoolrc"
      gSVersionIni = gtIniPath + ".sversionrc"
    else
      gTesttoolIni = gtIniPath + "testtool.ini"
      gSVersionIni = gtIniPath + "sversion.ini"
   end if

   if Dir (gTesttoolIni) = "" then
      if GUI <> 4 then
         MsgBox ("The Test ends, because 'testtool.ini' is missing at : "+ Chr(13) + gTesttoolIni, 1, "Config-file for testtool is missing")
       else
         MsgBox ("The Test ends, because '.testtoolrc' is missing at : " + Chr(13) + gTesttoolIni, 1, "Config-file for testtool is missing")
      end if
      end
   end if
'///Get <b>iGuiVersion</b> out of <i>testtool.ini/.testtoolrc</i>
   iGuiVersion = GetIniValue (gTesttoolIni, "GUI Platform", "Current")
   if iGuiVersion = 0 then
      if GUI <> 4 then
         iGuiVersion = 400
         qaErrorLog("The GUI-entry in '" + gTesttoolIni + "' is missing!"+Chr(13)+"Please insert the correct number for your operating system! In Section [GUI Platform], entry Current= "+Chr(13)+"Win95....: 100"+Chr(13)+"Win98....: 395"+Chr(13)+"WinNT....: 351 or 400"+Chr(13)+"WinSE....: 410"+Chr(13)+"WinME....: 490"+Chr(13)+"Win2000..: 500"+Chr(13)+"WinXP....: 501")
       else
         iGuiVersion = 01
         QAErrorLog("The GUI value in '" + gTesttoolIni + "' is missing!"+Chr(13)+"Please insert the correct number for your operating system! In Section [GUI Platform], entry Current= "+Chr(13)+"Solaris SPARC.: 01"+Chr(13)+"Linux.........: 03"+Chr(13)+"AIX...........: 04"+Chr(13)+"Solaris x86...: 05"+Chr(13)+"HP-UX.........: 07"+Chr(13)+"FreeBSD........: 08"+Chr(13)+"Mac OS X......: 12"+Chr(13)+"Linux PPC.....: 13"+Chr(13)+"NetBSD.....: 14"+Chr(13)+"Linux 64 bit.....: 15"+Chr(13)+"Linux SPARC.....: 16")
      end if
   end if
'///Get the variable <b>gSamePC</b>
   gHost = GetIniValue (gTesttoolIni, "Communication", "Host")
   if (gHost = "") then gHost = "LOCALHOST"                 ' if entry is not in configuration file set default value
   if uCase (gHost) = "LOCALHOST" then                             ' ein Client kann immer nur lokal sein
      gSamePC = TRUE
    else
      gSamePC = FALSE
   end if
'///Set <b>gtPlatform</b> and <b>gtSYSName</b>
   select case GUI
      case 1:                                      
      'Windows
         select case iGuiVersion
            case 100:   gSystemnummer = SYS_W95
            case 351:   gSystemnummer = SYS_WNT
            case 395:   gSystemnummer = SYS_W95
            case 400:   gSystemnummer = SYS_WNT
            case 410:   gSystemnummer = SYS_WSE
            case 490:   gSystemnummer = SYS_WME
            case 500:   gSystemnummer = SYS_W2K
            case 501:   gSystemnummer = SYS_WXP
            case else:  Print "Unknown OS ("+GUI+"/"+iGuiVersion+") please insert in sysinfo.inc::GetToolPlatform, WinNT will be used now."
                        gSystemnummer = SYS_WNT
         end select

      case 2:
      'eComStation
         select case iGuiVersion
            case 17: gSystemNummer = SYS_ECS 
            case else: Print "Unknown OS ("+GUI+"/"+iGuiVersion+") please insert in sysinfo.inc::GetToolPlatform, WinNT will be used now."
                       gSystemNummer = SYS_ECS
         end select
      case 3:
      'Deprecated OSes: - (2) OS/2 2.x, 3.x, 4 (Merlin), 4.5 (Aurora)
      '                 - (3) Mac OS Classic
                    print "Deprecated OS ("+GUI+"/"+iGuiVersion+") please insert in sysinfo.inc::GetToolPlatform."       
                    gSystemnummer = SYS_SOL
      case 4:                                      
      'UNIX
         select case iGuiVersion
            case 01 :   gSystemnummer = SYS_SOL
            case 03 :   gSystemnummer = SYS_LIN
            case 04 :   gSystemnummer = SYS_AIX
            case 05 :   gSystemnummer = SYS_X86
            case 07 :   gSystemnummer = SYS_HP
            case 08 :   gSystemnummer = SYS_FBSD
            case 12 :   gSystemnummer = SYS_OSX
            case 13 :   gSystemnummer = SYS_PPC
            case 14 :   gSystemnummer = SYS_NBSD
            case 15 :   gSystemnummer = SYS_LIN64
            case 16 :   gSystemnummer = SYS_LINSPARC
            case else:  Print "Unknown OS ("+GUI+"/"+iGuiVersion+") please insert in sysinfo.inc::GetToolPlatform. TestTool will try to use the same environment as Solaris SPARC uses."
                        gSystemnummer = SYS_SOL
         end select

   end select
   gtPlatform  =  PlatFormExt(gSystemnummer)
   gtSYSName   =  PlatFormName(gSystemnummer)
end sub

'-------------------------------------------------------------------------

sub GetOfficePlatform
'///<b>Get all information about the system where the office is running.</b>
'///The following global variables will be set:
'///+<ul><li><i>gPlatform</i>: Name of the operating system (3 digit e.g. 'w95')</li>
'///+<li><i>gSYSName</i>: Full name of the operating system</li>
'///+<li><i>gPlatGroup</i>: Groupname of the operation system (w95, unx)</li></ul>
'///If the Testtool and the office are running on the same machine (<b>gSamePC = TRUE</b>) then all variables have the same value.
'///+If not the value of <i>Aktuell</i> in section <i>[Office Platform]</i> has to be set in <i>testtool.ini/.testtoolrc</i> (testing remotely)
'///+the variables are :
'///+<TABLE BORDER=1><TR><TH>Name</TH><TH>Entry in <i>testtool.ini</i></TH></TR>
'///+<TR><TD>Win95</TD><TD>WIN95</TD></TR>
'///+<TR><TD>Win98</TD><TD>WIN98</TD></TR>
'///+<TR><TD>WinSE</TD><TD>WIN98SE</TD></TR>
'///+<TR><TD>WinME</TD><TD>WIN98ME</TD></TR>
'///+<TR><TD>WinNT</TD><TD>WINNT</TD></TR>
'///+<TR><TD>Win2000</TD><TD>WIN2000</TD></TR>
'///+<TR><TD>WinXP</TD><TD>WINXP</TD></TR>
'///+<TR><TD>Solaris SPARC</TD><TD>SOLARIS</TD></TR>
'///+<TR><TD>Linux</TD><TD>LINUX</TD></TR>
'///+<TR><TD>Solaris x86</TD><TD>X86</TD></TR>
'///+<TR><TD>Mac OS X</TD><TD>OSX</TD></TR>
'///+<TR><TD>Linux PPC</TD><TD>PPC</TD></TR>
'///+<TR><TD>AIX</TD><TD>AIX</TD></TR>
'///+<TR><TD>HP UNIX</TD><TD>HP</TD></TR>
'///+<TR><TD>FreeBSD</TD><TD>FBSD</TD></TR>
'///+<TR><TD>NetBSD</TD><TD>NBSD</TD></TR>
'///+<TR><TD>Linux 64 bit</TD><TD>LIN64</TD></TR>
'///+<TR><TD>Linux SPARC</TD><TD>LINSPARC</TD></TR>
'///+<TR><TD>eComStation</TD><TD>ecs</TD></TR>
'///+</TABLE>

  Dim GuiString as String
  Dim GUI as Integer

   if gSamePC = FALSE then
      GuiString = UCase (GetIniValue (gTesttoolIni, "Office Platform", "Current"))
      select case GuiString
         case "WIN95"   : GUI = 1
         case "WIN98"   : GUI = 2
         case "WIN98SE" : GUI = 3
         case "WIN98ME" : GUI = 4
         case "WINNT"   : GUI = 5
         case "WIN2000" : GUI = 6
         case "WINXP"   : GUI = 7
         case "SOLARIS" : GUI = 9
         case "LINUX"   : GUI = 10
         case "X86"     : GUI = 11
         case "OSX"     : GUI = 12
         case "PPC"     : GUI = 13         
         case "AIX"     : GUI = 15
         case "HP"      : GUI = 17        
         case "FBSD"    : GUI = 18        
         case "NBSD"    : GUI = 19        
         case "LIN64"   : GUI = 20
         case "LINSPARC": GUI = 21
         case "ECS"     : GUI = 22
         case else      : print "Unknown operating system : '" & GuiString & "'. The test run should be canceled."
                          exit sub
      end select
      gPlatform  =  PlatFormExt(GUI)
      gSYSName   =  PlatFormName(GUI)
      gPlatGroup =  PlatFormGroup(GUI)
   else
      gPlatform  =  gtPlatform
      gSYSName   =  gtSYSName
      gPlatGroup =  PlatFormGroup(gSystemnummer)
   end if
end sub

'-------------------------------------------------------------------------

function hGetSystemLanguage as Integer
'///Get the systemlanguage (ID)
  Dim sDummy as String  
   
   hGetSystemLanguage = 0

   try
      sDummy = GetSystemLanguage
   catch
      print "You are testing an Office, that wasn't synced at least with version srx 644r Build 8509! \n Please call the Developer, that provided the CWS to you and tell him to resync !!!!!!!!!!"
      end
   endcatch
  
   if sDummy = "" then
     warnlog "sysinfo.inc::hGetSystemLanguage: The Systemlanguage is unknown, the default will be 01 = English (USA)."
     hGetSystemLanguage = 1
    else 
     hGetSystemLanguage = ConvertCodeToLanguage (sDummy)
   end if
end function

'-------------------------------------------------------------------------

function hfGetTTProfileName as string
' Author: JSI, 22. January 2003
'/// Get the current profile name in the TestTool control file (testtool.ini / .testtoolrc). 
 hfGetTTProfileName = GetIniValue (gTesttoolIni, "Misc", "CurrentProfile")
end function 
