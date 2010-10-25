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
'* short description : starting routines for declare.bas
'*
'*****************************************************************
'*
' #1 GetUseFiles
' #1 LoadDeclaration
' #1 TestEnter
' #1 TestExit
' #1 hOfficeStarten
' #0 TrimTab 
' #0 lTrimTab 
' #0 rTrimTab 
'*
'\****************************************************************

' *****************************************************************
'  ruft alle wichtigen Startroutinen auf
'
sub GetUseFiles()
'///
   Use "global\system\includes\sysinfo.inc"
   Use "global\system\includes\inivalue.inc"
   Use "global\system\includes\iniinfo.inc"

'   Use "global\tools\includes\optional\t_xml1.inc"
   Use "global\tools\includes\required\t_tools1.inc"
   Use "global\tools\includes\required\t_tools2.inc"
   Use "global\tools\includes\required\t_lists.inc"

   Call LoadDeclaration                  ' Deklarationen einlesen ( master.inc )
   Call GetToolPlatform                  ' Plattform und Pfade des Testtools bestimmen ( sysinfo.inc )
   Call GetOfficePlatform                ' Plattform und Pfade des Office bestimmen ( sysinfo.inc )
   Call GetIniInformation                ' alle wichtigen Daten aus der Testtool.ini ziehen und auf glob. Variable setzen ( iniinfo.inc )
   Call GetOfficeInformation             ' set 'sAppExe' for startup
   Call hOfficeStarten                   ' try to start the office for first time ( master.inc )
   Call GetLanguageInformation           ' die Sprache des Office ermitteln und ggf. einstellen ( iniinfo.inc )
   Call hDisableQuickstarterAPI
end sub

' **********************************************************
'  Einbinden der Deklarationen
'
sub LoadDeclaration
'///load all sid-files
   use "global\sid\all.sid"         ' all menu items ( german )
   use "global\sid\bars.sid"         ' all bars as slots
   use "global\sid\context.sid"      ' slots only used in context menus
   use "global\sid\e_all.sid"        ' all menu items ( english )
   use "global\sid\others.sid"       ' general slots ( german )

'///load all win-files
   use "global\win\tab_a_d.win"      ' Tabpages TabA ...TabZ
   use "global\win\tab_e_g.win"
   use "global\win\tab_h_o.win"
   use "global\win\tab_p_s.win"
   use "global\win\tab_t_z.win"

   use "global\win\dial_a_c.win"     ' Dialogs A ... Z
   use "global\win\dial_d_h.win"
   use "global\win\dial_i_o.win"
   use "global\win\dial_p_s.win"
   use "global\win\dial_t_z.win"

   use "global\win\sys_dial.win"     ' Systemdialogs
   use "global\win\dokument.win"     '
   use "global\win\mathop.win"       ' Mathoperators

   use "global\win\piloten.win"      ' Draw-Autopilot
   use "global\win\w_autop.win"      ' Writer-Autopilots

   use "global\win\bars.win"         ' bars with buttons ( englisch )

   use "global\win\e_mathop.win"
   use "global\win\edia_a_c.win"
   use "global\win\edia_d_h.win"
   use "global\win\edia_i_o.win"
   use "global\win\edia_p_s.win"
   use "global\win\edia_t_z.win"
   use "global\win\etab_a_d.win"
   use "global\win\etab_e_g.win"
   use "global\win\etab_h_o.win"
   use "global\win\etab_p_s.win"
   use "global\win\etab_t_z.win"

   use "global\win\etoolbox.win"
   
   use "global\win\spadmin.win"
   use "global\win\reportdesigner.win"

end sub
'
'-------------------------------------------------------------------------------
'
sub TestEnter
'///special TestEnter : without recovering at starting a testcase

   Dim sFehler as String
   Dim Unterbrechnung as Boolean

   Unterbrechnung = FALSE

   try
      sFehler = ResetApplication
      if sFehler <> "" then
         WarnLog "   **    Fehler in der TestEnter-Routine      **"

         if InStr ( 1, sFehler, "nicht schliessen", 1 ) <> 0 then
            printLog sFehler
            Unterbrechnung = TRUE
         else
            printLog sFehler
         end if
      end if
   catch
      WarnLog "   **    Fehler in der TestEnter-Routine      **"
      printLog "!!!  Konnte nicht auf Grundzustand kommen  !!!"
      Unterbrechnung = TRUE
   endcatch

   try
      AppAbort                       ' Alle noch anstehenden Befehle loeschen
   catch
   endcatch

   if Unterbrechnung = TRUE then
      if ( MsgBox ("Konnte nicht in den Grundzustand kommen." + Chr(10) + "Bitte die Applikation selbst in den Basiszustand setzten und OK druecken", 1, "TestEnter-Routine" )) = 2 then end
   else
      Kontext "Standardbar"
      if Standardbar.Exists = FALSE then
         ViewFullScreen
         if Standardbar.Exists (3) then
            Warnlog "Ansicht/Ganzerbildschirm war noch eingeschaltet"
         else
            ViewToolbarsFunctionbar
            if Standardbar.Exists (3) then Warnlog "Standardbar was turned off"
         end if
      end if
   end if

   try
      AppAbort                       ' Alle noch anstehenden Befehle loeschen
   catch
   endcatch

   Kontext
end sub
'
'-------------------------------------------------------------------------------
'
sub TestExit
'///special TestEnter : without recovering at ending a testcase
   Dim sFehler as String
   Dim Unterbrechnung as Boolean

   Unterbrechnung = FALSE

   try
      sFehler = ResetApplication
      if sFehler <> "" then
         Warnlog "   **    Fehler in der TestExit-Routine      **"

         if InStr ( 1, sFehler, "nicht schliessen", 1 ) <> 0 then
            printLog sFehler
            Unterbrechnung = TRUE
         else
            printLog sFehler
         end if
      end if
   catch
      WarnLog "   **    Fehler in der TestExit-Routine      **"
      printLog "!!!  Konnte nicht auf Grundzustand kommen, es wurde vielleicht von Hand recovert.   !!!"
      Unterbrechnung = TRUE
   endcatch

   try
      AppAbort                       ' Alle noch anstehenden Befehle loeschen
   catch
   endcatch

   if Unterbrechnung = TRUE then
      if ( MsgBox ("Konnte nicht in den Grundzustand kommen." + Chr(10) + "Bitte die Applikation selbst in den Basiszustand setzten und OK druecken", 1, "TestExit-Routine" )) = 2 then End
   else
      Kontext "Standardbar"
      if Standardbar.Exists = FALSE then
         ViewFullScreen
         if Standardbar.Exists (3) then
            Warnlog "Ansicht/Ganzerbildschirm war noch eingeschaltet"
         else
            ViewToolbarsFunctionbar
            if Standardbar.Exists (3) then Warnlog "Standardbar was turned off"
         end if
      end if
   end if

   try
      AppAbort                       ' Alle noch anstehenden Befehle loeschen
   catch
   endcatch

   Kontext
end sub

'-------------------------------------------------------------------------------

sub hOfficeStarten
    dim sUnoPort as string
    dim sPlatformProgramPath as string
    dim sPlatformBinExt as string

    ' Set a valid URE_BOOTSTRAP path for soffice, else the invalid one from this testtool instance
    ' will get used i86718 - would result in a message about vcl stuff and a not starting soffice
    if gPlatform = lcase("osx") then
        sPlatformProgramPath = "MacOS"
        sPlatformBinExt = "rc"
    else
        sPlatformProgramPath = "program"
        if gPlatGroup <> "unx" then
            sPlatformBinExt = ".ini"
        else
            sPlatformBinExt = "rc"
        end if
    end if
    setChildEnv("URE_BOOTSTRAP",convertToURL(convertPath(gNetzOfficePath + sPlatformProgramPath + "/fundamental" + sPlatformBinExt)))


    if gClient = TRUE then
       try
          start sAppExe, sAppParameter
' hiermit kann der Client nicht gestartet werden, aber wenn er schon aktiv ist, darf der Shell-befehl nicht ausgeführt werden
       catch
          shell ( sAppExe & sAppParameter, 1 )
          wait 10000
       endcatch
    end if

    hGetUNOService(true, sUnoPort)
    try
       start sAppExe, "-enableautomation -norestore -nolockcheck -accept=socket,host=localhost,port=" + sUnoPort + ";urp "
    catch
       if gSamePC = FALSE then
          Exceptlog
          if MsgBox ("Ein Office auf einem anderen Rechner/Plattform kann nicht automatisch gestartet werden." + Chr(10) + "Bitte das Office selber starten und Test weiterlaufen lassen.", 1, "Starten des Office" ) = 2 then end
       else
          Exceptlog
          if gClient = TRUE then
             if MsgBox ( "Beim Start des Office-Client gab es Probleme. Bitte den Client von Hand starten und den Test weiterlaufen lassen!", 1, "Starten des Office" ) = 2 then end
          else
             if gNetzInst = TRUE then
                Warnlog "Beim Start des Workstation-Office gab es Probleme, es wird ein 2. Start versucht!"
             else
                Warnlog "Beim Starten des Office gab es Probleme, es wird ein 2. Start versucht!"
             end if
          end if
       end if
       wait 2000
   endcatch

   if gClient = TRUE then
      start sAppExe, sAppParameter
   else
      start sAppExe, "-enableautomation -norestore -nolockcheck "
   end if
   wait 3000
end sub

'-------------------------------------------------------------------------------

function TrimTab ( sTrimmer as String ) as String
   Dim sInterim as String

   sInterim = sTrimmer
   sInterim = lTrimTab ( sInterim )
   TrimTab = rTrimTab ( sInterim )

end function

'-------------------------------------------------------------------------------

function lTrimTab ( slTrimmer as String ) as String
   Dim i, iLen as Integer
   Dim sInterim as String

   iLen = len ( slTrimmer )
   sInterim = slTrimmer

   for i=1 to iLen
      if Asc ( left ( sInterim, 1 ) ) = 9 then
         sInterim = Right ( sInterim, len ( sInterim ) - 1 )
      else
         i=iLen+1
      end if
   next i
   lTrimTab = sInterim
end function

'-------------------------------------------------------------------------------

function rTrimTab ( srTrimmer as String ) as String
   Dim i, iLen as Integer
   Dim sInterim as String

   iLen = len ( srTrimmer )
   sInterim = srTrimmer

   for i=1 to iLen
      if Asc ( right ( sInterim, 1 ) ) = 9 then
         sInterim = left ( sInterim, len ( sInterim ) - 1 )
      else
         i=iLen+1
      end if
   next i
   rTrimTab = sInterim
end function

'-------------------------------------------------------------------------------

sub StartTheOffice
    '/// this routine is not intended to be called from within a testscript.
    '/// this routine will get called by the VCL TestTool application, in case it recognizes there is no soffice.bin running anymore or not responding anymore
    '/// this happens usually after OpenOffice.org crashed
    'keep it as short as possible! 
    warnlog "OpenOffice.org application crashed or can not be started. Double click the above error message if exists for the culprint."
    hOfficeStarten()
    try
        catchGPF false
    catch
    endcatch
    hDisableQuickstarterAPI()
    ' set global variable for backwards compatibility
    gStartTheOffice = TRUE
end sub

