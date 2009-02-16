'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: t_tools1.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-13 10:27:11 $
'*
'* This file is part of OpenOffice.org.
'*
'* OpenOffice.org is free software: you can redistribute it and/or modify
'* it under the terms of the GNU Lesser General Public License version 3
'* only, as published by the Free Software Foundation.
'*
'* OpenOffice.org is distributed in the hope that it will be useful,
'* but WITHOUT ANY WARRANTY; without even the implied warranty of
'* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
'* GNU Lesser General Public License version 3 for more details
'* (a copy is included in the LICENSE file that accompanied this code).
'*
'* You should have received a copy of the GNU Lesser General Public License
'* version 3 along with OpenOffice.org.  If not, see
'* <http://www.openoffice.org/license.html>
'* for a copy of the LGPLv3 License.
'*
'/************************************************************************
'*
'* owner : helge.delfs@sun.com
'*
'* short description : Tools (1)
'*
'***************************************************************************************
'*
' #1 GetClipboardText                   'Returns the correct clipboard text (also if there is a 'RETURN' at it's end.
' #1 hDoubleClickInList
' #0 hMouseClick
' #1 wielange
' #1 sleep
' #1 WaitInSek
' #1 WaitInMilliSek
' #1 DialogTest                         'Creates snapshots
' #0 DialogTest2                        'Creates snapshots
' #1 hFindeImDokument                   'Searches for a string with the 'Search&Replace'-dlg in a document.
' #1 hFindeMehrImDokument               'Same as 'hFindeImDokument' but needs an integer how often the search-phrase must be found until the boolean gives TRUE back.
' #1 ErgebnisSchreiben
' #1 TextInDatei
' #1 hGetUIFiltername                   'Extracts the UI filtername from configuration.
' #1 hGetFilternameExtension            'Extracts the filtername extension from configuration.
' #1 TrimTab                            'trimming strings
' #1 lTrimTab                           'trimming strings
' #1 rTrimTab                           'trimming strings
' #1 TrimString                         'Cuts all ASCII characters which are defined by a parameter (e.g. 32 space will be deleted)
' #1 ActiveDeactivateAsianSupport       'Acivates/Deactivates the Asian support in StarOffice
' #1 ActiveDeactivateCTLSupport         'Acivates/Deactivates the CTL support in StarOffice
' #1 GetDecimalSeperator                'Reads from Tools / Options the used . or , which is being used as seperator
' #1 sResetTheOffice                    'Save language information - throw away user directory - restore language information
'*
'\*************************************************************************************

function GetClipboardText as string
'/// Returns the correct clipboard text (also if there is a 'RETURN' at it's end.
   Dim i% : Dim CBText$
   Dim Zwischen$

   wait 500
   GetClipboardText = ""
   CBText$ = GetClipboard

   if CBText$ = "" then
      GetClipboardText = ""
      exit function
   end if

   if asc ( Right( CBText$, 1 )) = 10 then
      Zwischen$ = Mid( CBText$, 1, len(CBText$)-1 )
      if Zwischen$ <> "" then
         if asc ( Right( Zwischen$, 1 )) = 13 then
            GetClipboardText = Mid( Zwischen$, 1, len(Zwischen$)-1 )
         else
            GetClipboardText = Zwischen$
         end if
      else
         GetClipboardText = Zwischen$
      end if
   else
      if asc ( Right( CBText$, 1 )) = 13 then
         Zwischen$ = Mid( CBText$, 1, len(CBText$)-1 )
         if asc ( Right( Zwischen$, 1 )) = 10 then
            GetClipboardText = Mid( Zwischen$, 1, len(Zwischen$)-1 )
         else
            GetClipboardText = Zwischen$
         end if
      else
         GetClipboardText = CBText$
      end if
   end if
end function

'-------------------------------------------------------------------------

function hDoubleClickInList ( window, Selektion as String, optional bFocus as boolean ) as Boolean
'/// hDoubleClickInList
'///+ Makes a double click onto an entry in a list (tested only in <i>style lists</i>)
'///+ window: name of list ///'
'///+ selektion: string to find in list ///'
'///+ bFocus: TRUE: activate the window with mouseclick before leaving ///'
'///+ ReturnValue: if found: TRUE; else FALSE ///'

    Dim i as Integer
    Dim AlterWert as String
    Dim NeuerWert as String

    NeuerWert = "!=!  !=!"                ' init with dummy value
    window.TypeKeys "<Home>"
    if window.gettext <> Selektion then         
        for i=1 to 100 step 2
            window.MouseDown 5, i +1
            window.MouseUp 5, i +1
            AlterWert = window.GetText
            window.TypeKeys "<Down>"
            NeuerWert = Window.GetText
            window.TypeKeys "<Up>"
            if AlterWert = Selektion then
                window.MouseDown 5, i +1
                window.MouseUp 5, i +1
                if Window.GetText = Selektion then     ' catch if <down> had any effects
                    window.MouseDoubleClick 5, i +1
                    if (isMissing (bFocus) = FALSE) then ' if optional parameter provided
                        window.MouseDown 5, i +1
                        window.MouseUp 5, i +1
                    endif
                    i = 202
                else
                    i=0                                 ' start at top of list
                end if
            else
                if AlterWert = NeuerWert then
                    Warnlog "'" + Selektion + "' wasn't found in list!"
                    i = 202
                else
                    if i > 98 then i=40                  ' list not at end, but scrolled
                end if
            end if
        next i
        if i < 200 OR i > 100 then
            hDoubleClickInList = FALSE
        else
            hDoubleClickInList = TRUE
        end if
    else
        window.TypeKeys "<Return>"
        hDoubleClickInList = TRUE
    endif
end function

'-------------------------------------------------------------------------

sub hMouseClick ( window, xPos, yPos )
' Author: Thorsten Ziehm (26.09.2000)
'/// hMouseClick
'///+ Do a mouse click on a named window.
'/// <i>Input</i>:
'///+ window : The object on which the mouse click should be make (document, listbox, window)
'///+ xPos : x-position (relativ to the size of the window (1:100)
'///+ yPos : y-position (relativ to the size of the window (1:100)
   window.MouseDown ( xPos, yPos )
   window.MouseUp ( xPos, yPos )
end sub

'-------------------------------------------------------------------------

function wielange (StrtTime, optional iFormat as integer) as String
    ' Author: Michael Friedrichs
    '/// wielange
    '///+ Returns the time between a start- and an end timeframe.
    '///+  iFormat: 0: default; 1: mysql ///'
    
    Dim Zeitspanne
    Dim Zeitspannesek
    Dim Zeitspannemin
    Dim Zeitspanneh
    dim sTemp as string
    
    if isMissing(iFormat) then
        'dim iFormat as integer
        iFormat = 0
    endif
    
    Zeitspanne = Now() - StrtTime
    Zeitspannesek = Zeitspanne / 1.15741E-05 + 1
    Zeitspanneh = Fix(Zeitspannesek / 3600)
    Zeitspannesek = Zeitspannesek - Zeitspanneh * 3600
    Zeitspannemin = Fix(Zeitspannesek / 60)
    Zeitspannesek = Zeitspannesek - Zeitspannemin * 60
    Zeitspannesek = Fix(Zeitspannesek)
    select case iFormat
        case 0: sTemp = "" & Zeitspanneh & "h " & Zeitspannemin & "m " & Zeitspannesek & "s"
        case 1: if Zeitspanneh < 10 then         ' mysql format for status.inc
                    sTemp = "0" & Zeitspanneh & ":"
                else
                    sTemp = "" & Zeitspanneh & ":"
                end if
                if Zeitspannemin < 10 then
                    sTemp = "" & sTemp & "0" & Zeitspannemin & ":"
                else
                    sTemp = "" & sTemp & Zeitspannemin & ":"
                end if
                if Zeitspannesek < 10 then
                    sTemp = "" & sTemp & "0" & Zeitspannesek
                else
                    sTemp = "" & sTemp & Zeitspannesek
                end if
        case default: qaErrorLog "t_tools1.inc::wielange: optional parameter iFormat out of range!"
                      sTemp = ""
    end select
    wielange = sTemp
end function

'-------------------------------------------------------------------------

sub WaitInSek ( Sekunden )
' Author: Thorsten Ziehm
'/// WaitInSek
'///+ Wait exactly x second(s) (using GetSystemTicks)
  Dim i : Dim t0 : Dim t1
   t0 = GetSystemTicks()
   for i=1 to 10000*Sekunden
      t1 = GetSystemTicks()
      if t1-t0 > 1000*Sekunden then i=11000*Sekunden
   next i
end sub

'-------------------------------------------------------------------------

sub WaitInMilliSek ( Milli )
' Author: Thorsten Ziehm
'/// WaitInMilliSek
'///+ Wait exactly x millisecond(s) (using GetSystemTicks)
  Dim i : Dim t0 : Dim t1
   t0 = GetSystemTicks()
   for i=1 to 1000*Milli
     t1 = GetSystemTicks
     if t1-t0 > Milli then i=1001*Milli
   next i
end sub

'-------------------------------------------------------------------------

sub sleep ( i% )
'/// sleep
'///+ simple sleep routine which uses seconds.
'   WaitInSek ( i% )
   wait i%*1000
end sub

'-------------------------------------------------------------------------

sub DialogTest( Window, optional iNumber as integer)
'/// DialogTest
'///+ Make <i>SnapShots</i>
'/// <b>Window</b> : the name of the window as declared in qa/qatesttool/global/win/*
'/// <i>Optional Parameter</i> <b>iNumber</b> : Number to distinguish windows which dynamical change their content but not their ID///'
'///+ the number has to be provided by the testscript creator ///'
   Dim Ergebnis as Integer
   Dim Ausgabe as String
   Dim UndRaus as Boolean
   Dim sCount as string

   ' evaluate optional parameter
   if isMissing(iNumber) then
        'just one picture
        sCount = ""
   else
        'there will be more pictures with the same ID
        sCount = "_"+iNumber
   endif
   
   if gDasNicht=0 then
      ' In Place Translation Feature: not used anymore;
      ' The matching of the strings on the later migration step never worked.
      ' Just kept here for historical reasons
      Ausgabe = ""
      UndRaus = FALSE
      while UndRaus = FALSE
         Ausgabe = translate
         if Ausgabe <> "" OR Ausgabe <> "1" then
            if Left ( Ausgabe, 1 ) = "0" then
               Ausgabe = Right ( Ausgabe, Len( Ausgabe )- 2 )
               AnhaengenAnDatei ( gOfficePath + "trans_output.txt", Ausgabe )
            end if
         end if
         if Ausgabe = "1" then UndRaus = TRUE
      wend
   else
      ' Usual window check
      try
         if Not window.Exists(2) then
            Warnlog "   - Window nicht existent:" + window.Name + " " + window.ID
            exit sub
         end if
         'To get a history, of what windows are covered, use the following line
         '      AnhaengenAnDatei ( ConvertPath (gOfficePath + "user\work\wieviel.txt"), window.Name + " " + sCount + " : " + window.ID )
      catch
         ExceptLog
      endcatch
   end if

   if gbSnapShot = TRUE then
      'Make Screenshot from dialog and save as HelpID.bmp
      Dim Dummy as String, sName as String, sPicName as String

      'get window ID
      Dummy = Window
      'set filename
      sName = Dummy + sCount + ".bmp"

      'save with respect to application and language
      sCapturePath = ConvertPath (gOfficePath + "user\work\screenshots"+iSprache+"\")
      sPicName = sCapturePath + lCase(gApplication)
      'create directory if it doesn't exist
      if hDirectoryExists(sPicName) <> TRUE then
            mkdir (sPicName)
      end if
      sPicName = sPicName + sName
      try
        sleep 1
        window.SnapShot( sPicName )
      catch
        warnlog "t_tools1.inc::DialogTest Failed to save screenshot: '" + sPicName + "'"
      endcatch
      printlog sPicName
   end if
end sub

'-----------------------------------------------------------

sub DialogTest2( Window, i% )
    'deprecated TBO:2006/03/16
    DialogTest( Window, i% )
end sub

'-------------------------------------------------------------------------

function hFindeImDokument ( Passage$ , Optional A, optional bRegEx ) as boolean
' Author: Joerg Sievers (13.11.2001)
'/// hFindeImDokument
'/// Searches via 'Search&Replace'-Dlg in StarOffice Writer, -Clac,
'///+ -HTML, -GlobalDoc for the string <b>EXACT MATCH</b>.
'///+ Only ONE TIME and THE FIRST search phrase will be found!
'/// <i>Optional Parameter</i> <b>a</b> : If you do not want a warnlog message
'/// <i>Optional Parameter</i> <b>bRegEx</b> : if you look fort an regular expression
 Dim WhatIsIn as string
 Dim bSilent as boolean
 
   bSilent = NOT isMissing(a)
   gApplication = UCase ( gApplication )
   hFindeImDokument = FALSE

   select case gApplication

      case "CALC"        : Kontext "DocumentCalc"
                           DocumentCalc.TypeKeys "<MOD1 HOME>"

      case "WRITER"      : Kontext "DocumentWriter"
                           DocumentWriter.TypeKeys "<MOD1 HOME>"

      case "NACHRICHT"   : Kontext "DokumentNachrichten"
                           DokumentNachrichten.TypeKeys "<MOD1 HOME>"

      case "HTMLDOKUMENT": Kontext "DocumentWriterWeb"
                           DocumentWriterWeb.TypeKeys "<MOD1 HOME>"

      case "GLOBALDOC"   : Kontext "DocumentMasterDoc"
                           DocumentMasterDoc.TypeKeys "<MOD1 HOME>"

   end select

  SetClipboard ""
  EditSearchAndReplace
  Kontext "FindAndReplace"
  if SimilaritySearch.IsVisible = False then
    More.Click
  end if 
  if MatchCase.IsChecked = False then
    MatchCase.Check
  end if
  if SimilaritySearch.IsChecked = TRUE then
    SimilaritySearch.UnCheck
    if NOT bSilent then    
        warnlog "'Similarity search' checkbox was enabled! It shouldn't be the default!"
    endif
  end if
  if IsMissing(bRegEx) <> TRUE then
    RegularExpressions.Check
  end if
  SearchFor.Settext Passage$
  SearchNow.Click
  Kontext
  if NOT Active.Exists(2) then
      Kontext "FindAndReplace"
    More.Click 
    FindAndReplace.Cancel
     EditCopy
     WhatIsIn = GetClipboardText
     if WhatIsIn <> Passage$ then
      if NOT bSilent then
       warnlog "The search-request for '" & Passage$ & "' has been fault! (Found: '"& WhatIsIn & "')"
      end if
      else
       hFindeImDokument = TRUE
     end if
  else
   try
      Kontext
       if Active.Exists(1) then 
                      Active.OK
       end if
       if NOT bSilent then
           warnlog "The search-request for '" & Passage$ & "' has been fault! (Found: '"& WhatIsIn & "')"           
       end if
       Kontext "FindAndReplace"
       if SimilaritySearch.IsVisible = False then
        More.Click
       endif
       if MatchCase.IsChecked then
        MatchCase.UnCheck
       endif
       if SimilaritySearch.IsChecked = TRUE then
            SimilaritySearch.UnCheck
            if NOT bSilent then
                warnlog "'Similarity search' checkbox was enabled! It shouldn't be the default!"
            endif
       end if
       if IsMissing(bRegEx) <> TRUE then
        RegularExpressions.UnCheck
       endif
       More.Click
       FindAndReplace.Cancel
    catch
      Active.Yes
       Kontext
        if bSilent then
           if Active.Exists then printlog "> "+Active.GetText
        endif
        if Active.Exists then Active.OK
        if NOT bSilent then
           warnlog "The search-request for '" & Passage$ & "' has been fault! (Found: '"& WhatIsIn & "')"
        endif
        Kontext "FindAndReplace"
        if SimilaritySearch.IsVisible = False then
            More.Click
        endif
        if MatchCase.IsChecked then
            MatchCase.UnCheck
        endif
        if SimilaritySearch.IsChecked = TRUE then
            SimilaritySearch.UnCheck
            if NOT bSilent then
                warnlog "'Similarity search' checkbox was enabled! It shouldn't be the default!"
            endif
        end if
        if IsMissing(bRegEx) <> TRUE then
            RegulaererAusdruck.UnCheck
        endif
        More.Click
        FindAndReplace.Cancel
    endcatch
  end if
end function

'-------------------------------------------------------------------------

function hFindeMehrImDokument ( Passage as string , WieOft as integer ) as boolean
' Author: Joerg Sievers (26.07.2000)
'/// hFindeMehrImDokument
'/// Searches per 'Search&Replace'-Dlg in StarOffice Writer, -Clac,
'///+-HTML, -GlobalDoc for the string <b>EXACT MATCH</b>.
'/// You have to give the function the number how often the phrase
'///+should be found in the document as an additional parameter (as integer).
'/// Only when exact the number of the phrase will be found correctly
'///+the function gives back TRUE.
'/// <i>see also</i> : hFindeImDokument (TOOLS.INC)
   Dim i as integer
   gApplication = UCase ( gApplication )

   hFindeMehrImDokument = FALSE

   select case gApplication

      case "CALC"        : Kontext "DocumentCalc"
                           DocumentCalc.TypeKeys "<MOD1 HOME>"

      case "WRITER"      : Kontext "DocumentWriter"
                           DocumentWriter.TypeKeys "<MOD1 HOME>"

      case "NACHRICHT"   : Kontext "DokumentNachrichten"
                           DokumentNachrichten.TypeKeys "<MOD1 HOME>"

      case "HTMLDOKUMENT": Kontext "DocumentWriter"
                           DocumentWriter.TypeKeys "<MOD1 HOME>"

      case "GLOBALDOC"   : Kontext "DocumentMasterDoc"
                           DocumentMasterDoc.TypeKeys "<MOD1 HOME>"

   end select

   SetClipboard ""
   EditSearchAndReplace

 For i = 1 to WieOft
  Kontext "FindAndReplace"
   if NOT MatchCase.IsChecked then MatchCase.Check
   SearchFor.Settext Passage
   SearchNow.Click
  Kontext
   if NOT Active.Exists(2) then
     FindAndReplace.Cancel
      EditCopy
      if GetClipboardText <> Passage then
        warnlog "The search-request for '" & Passage & "' has been fault!"
        i = WieOft
       else
        if i = WieOft then
          hFindeImDokument = TRUE
          printlog "Searchphrase found " & i & " time(s)."
        end if
      end if
    else
     try
       Active.OK
        Kontext
         if Active.Exists then Active.OK
         warnlog "The search-request for '" & Passage & "' has been fault!"
          i = WieOft
          Kontext "FindAndReplace"
           if MatchCase.IsChecked then MatchCase.UnCheck
           FindAndReplace.Cancel
      catch
        Active.Yes
         Kontext
          if Active.Exists then Active.OK
          warnlog "The search-request for '" & Passage & "' has been fault!"
          i = WieOft
          Kontext "FindAndReplace"
           if MatchCase.IsChecked then MatchCase.UnCheck
           FindAndReplace.Cancel
      endcatch
   end if
  Next i
  Kontext "FindAndReplace"
   if FindAndReplace.Exists(2) then
     FindAndReplace.Cancel
   end if
end function

'-------------------------------------------------------------------------

sub ErgebnisSchreiben ( Window, Name$ )
'/// ErgebnisSchreiben
'///+ Used in context with making screenshots.
  Dim FileNum% : Dim i%
  Dim Datei$
  Dim Text$ : Dim Text2$

  Datei$ = sCapturePath + "Ergebis.txt"
  Text2$ = Window.Name
  Text$ = Text2$ +  "    =>   " + Name$

  FileNum% = FreeFile
  Open Datei$ for Append  as #FileNum%
  Print #FileNum%, Text$
  Close #FileNum%
end sub

'-------------------------------------------------------------------------

sub TextInDatei ( TextText$, Datei$ )
'/// TextInDatei
  Dim FileNum%

  FileNum% = FreeFile
  Open Datei$ for Append  as #FileNum%
  Print #FileNum%, TextText$
  Close #FileNum%
end sub

'-------------------------------------------------------------------------

function hGetUIFiltername( vFiltername as string ) as string
'/// Returns the in the UI used filter name.
'///+ <b>INPUT</b>: 'internal', language independent filter name from FilterFactory.
'///+ <u>Examples</u>:<ul><li>hGetUIFiltername(&quot;StarOffice XML (Draw)&quot;) - Draw OOo 1.x/SO6.0/SO7 UI Filtername</li>
'///+ <li>sUIFiltername = hGetUIFiltername(&quot;StarOffice XML (Impress)&quot;) - Impress OOo 1.x/SO6.0/SO7 UI Filtername</li></ul>
'/// The 'internal' name can be found in the *.xcu in 
'///+ ..../share/registry/res/en-US/org/openoffice/TypeDetection/Filter.xcu.
'/// See also: hGetFilternameExtension
    Dim i as integer 
    Dim oOpenUNOService as object
    Dim oFilterName as object
    Dim oUno as object

    oUno = hGetUNOService(TRUE)
    
    oOpenUNOService = oUno.createInstance("com.sun.star.document.FilterFactory")
      try
          oFilterName = oOpenUNOService.getByName(vFiltername)

          for i=0 to ubound(oFilterName)
            if oFilterName(i).Name = "UIName" then
                          hGetUIFiltername = oFilterName(i).Value
                  end if       
          next i
      catch
          warnlog "t_tools1.inc::hGetUIFiltername('" + vFiltername + "'): Filtername is not available."
          hGetUIFiltername = ""
      endcatch
end function

'-------------------------------------------------------------------------

function hGetFilternameExtension ( vFilterName as string)
'/// Returns the in the UI used filter name extension(s) <u>as an <b>array</b></u>. 
'///+ <b>Important</b>: Also returns it <u>as an array</u> if there comes a string from the UNO API call.
'/// <u>Input</u>: 'internal', language independent name
'/// The 'internal' name can be found in the *.xcu in 
'///+ ../share/registry/modules/org/openoffice/TypeDetection/Types/fcfg_[Application_name]_types.xcu file(s).
'/// List of some 'internal' filter names for OOo 2.0/SO8:
'///+<TABLE BORDER=1><TR><TH>Filter</TH><TH><i>internal</i> name</TH><TH>Note</TH></TR>
'///+<TR><TD>Spreadsheet (default)</TD><TD>calc8</TD><TD>-</TD></TR>
'///+<TR><TD>Text document (default)</TD><TD>writer8</TD><TD>-</TD></TR>
'///+<TR><TD>Master document (default)</TD><TD>writerglobal8</TD><TD>-</TD></TR>
'///+<TR><TD>Drawing (default)</TD><TD>draw8</TD><TD>-</TD></TR>
'///+<TR><TD>Presentation (default)</TD><TD>impress8</TD><TD>-</TD></TR>
'///+<TR><TD>Formula/Math (default)</TD><TD>math8</TD><TD>-</TD></TR>
'///+<TR><TD>HTML</TD><TD>writer_web_HTML</TD><TD>two extensions!</TD></TR>
'///+<TR><TD>Text</TD><TD>writer_text</TD><TD>-</TD></TR>
'///+<TR><TD>StarWriter 5.0</TD><TD>writer_StarWriter_50</TD><TD>-</TD></TR>
'///+<TR><TD>StarCalc 5.0</TD><TD>calc_StarCalc_50</TD><TD>-</TD></TR>
'///+</TABLE><br>
' (rewritten, compatible routine; July 2004)
    Dim i as integer 
    Dim x as integer
    Dim oOpenUNOService as object
    Dim oFilterNameExtension as object
    Dim oUno as object    
    dim a as integer
    Dim aExtensions() as string
 
    'Initializize UNO comminication
    oUno = hGetUNOService(TRUE)
    
    'Using the TypeDetection service
    oOpenUNOService = oUno.createInstance("com.sun.star.document.TypeDetection")
      'Getting the Extension by given (internal; language- and product 
    'independent) filter name
    oFilterNameExtension = oOpenUNOService.getByName(vFiltername)

    'using ubound to count the nodes
      for i=0 to ubound(oFilterNameExtension)
              'if the node name is 'Extensions'...
        if oFilterNameExtension(i).Name = "Extensions" then
            '...if it's an array...
            if IsArray(oFilterNameExtension(i).Value) then
                'create dimension of the integer a 
                a = 10
                're-dimension the array with the integer a                
                Redim aExtensions(a) as string
                'return the array into an array
                aExtensions() = oFilterNameExtension(i).Value()                
            else            
                '...otherwise 'build' an array with only
                'one entry in (0)
                Redim aExtensions(0) as string                
                aExtensions(0) = oFilterNameExtension(i).Value                
            end if
              endif
      next i    
    'put the results into the return value of this function into an array.
    hGetFilternameExtension = aExtensions()
end function

'-------------------------------------------------------------------------

function TrimTab ( sTrimmer as String ) as String
'/// TrimTab
'/// <u>Input</u>: the original text
'/// Returns the string without &lt;tab&gt;s at the beginning and the end of a string.
   Dim sInterim as String

   sInterim = sTrimmer
   sInterim = lTrimTab ( sInterim )
   TrimTab = rTrimTab ( sInterim )

end function

'-------------------------------------------------------------------------

function lTrimTab ( slTrimmer as String ) as String
'/// lTrimTab
'/// <u>Input</u>: the original text
'/// Returns the string without &lt;tab&gt;s at the beginning.
'/// Cuts &lt;Tab's&gt; at the beginning of a string ( left )
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

'-------------------------------------------------------------------------

function rTrimTab ( srTrimmer as String ) as String
'/// rTrimTab
'/// Input: the original text
'/// Returns the string without &lt;tab&gt;s at the end.
'/// Cuts &lt;Tab's&gt; at the beginning of a string ( right )

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

'-------------------------------------------------------------------------

function TrimString (Content as String, delim as integer) as String
' Author: Frank Heitbrock (26.07.2002)
'/// TrimString
'/// <u>Input</u>: The  String, the delimiter which should be cut from the string.
'/// Returns the String without the delimiter.
'/// <u>Example</u>:
'///+ Content = &quot;&nbsp;H&nbsp;a&nbsp;l&nbsp;l&nbsp;o&nbsp;&quot;, delim = 32 (ascii for space character)
'///+ Return = &quot;Hallo&quot;
   dim strlen as integer, i as integer, k as integer
   dim CharBuff(1 to 100) as String
   dim ResultStr as String
   ' at first cut the empty strings left and right of the String
   Content = lTrim(Content)
   Content = rTrim(Content)
   ' now we search for all appropriate ascii characters in the middle of the String and delete them
   strlen = len(Content)
   k = 1
   for i = 1 to strlen
      if mid(Content, i, 1) <> chr(delim) then
         CharBuff(k) = mid(Content, i, 1)
         k = k +1
      end if
   next i
   for i = 1 to k
      ResultStr = ResultStr + CharBuff(i)
   next i
   TrimString = ResultStr

end function

'-------------------------------------------------------------------------

function ActiveDeactivateAsianSupport ( WhatState as Boolean ) as Boolean
' Author: Thorsten Ziehm
'/// ActiveDeactivateAsianSupport
'/// <u>Input</u>: TRUE or FALSE
'///+ TRUE: The Asian support will be enabled.
'///+ FALSE:  The Asian support will be disabled.
'/// <u>Return:</u>
'///+ TRUE/FALSE for the last state of the checkbox in the office UI.
   ToolsOptions
   hToolsOptions ( "LanguageSettings", "Languages" )

   IF Aktivieren.IsEnabled then        'the checkbox is disabled in asian versions
       ActiveDeactivateAsianSupport = Aktivieren.IsChecked         ' the function gets the old state of the checkbox
    
       if WhatState = TRUE then
          try
              Aktivieren.Check
          catch       
          endcatch      
       else
          Aktivieren.UnCheck
       end if
       gAsianSup = WhatState    ' Set the global variable
    
       Kontext "ExtrasOptionenDlg"
       ExtrasOptionenDlg.OK
       Sleep (3)
   else
       ActiveDeactivateAsianSupport = TRUE
       If WhatState = FALSE then
           warnlog "Deactivating of asian language support is not possible, because it is disabled in cjk versions"
       end if
       Kontext "ExtrasOptionenDlg"
       ExtrasOptionenDlg.OK
       Sleep (3)
   end if

end function

'-------------------------------------------------------------------------

function ActiveDeactivateCTLSupport ( WhatState as Boolean ) as Boolean
' Author: Hercule Li (March 2004)
'/// ActiveDeactivateCTLSupport
'/// <u>Input</u>: TRUE or FALSE
'/// TRUE :  The CTL will be enabled.
'/// FALSE:  The CTL will be disabled.
'/// <u>Return:</u>
'/// TRUE/FALSE for the last state of the checkbox in the office UI.
   ToolsOptions
   hToolsOptions ( "LanguageSettings", "Languages" )

    IF ComplexScriptEnabled.IsEnabled then        'the checkbox is disabled in CTL versions    
       ActiveDeactivateCTLSupport = ComplexScriptEnabled.IsChecked      ' the function gets the old state of the checkbox
    
       if WhatState = TRUE then
           ComplexScriptEnabled.Check
       else
           ComplexScriptEnabled.UnCheck
       end if
       gCTLSup = WhatState    ' Set the global variable
    
       Kontext "ExtrasOptionenDlg"
       ExtrasOptionenDlg.OK
       Sleep (3)
    else
       ActiveDeactivateCTLSupport = TRUE
       If WhatState = FALSE then
           warnlog "Deactivating of CTL language support is not possible, because it is disabled in ctl versions"
       end if
       Kontext "ExtrasOptionenDlg"
       ExtrasOptionenDlg.OK
       Sleep (3)
   end if

end function

'-------------------------------------------------------------------------

function GetDecimalSeperator ( optional sDummy$ ) as String
'/// <u>Precondition</u>: Measuring unit has to be set to centimeter (cm) before using this function. (see: fSetMeasurementToCM())    
'///+ <u>Input</u>: Number with fractionmark from <i>NumericField</i> as string     
'///+ <u>Output</u>: A dot (.) or a comma (,) as string
    Dim sCheckForSeparator as string
    Const cWhereIsThisFunction = "qa::qatesttool::global::tools::inc::t_tools1.inc::GetDecimalSeperator: " 
    Dim bDotOrCommaIncluded as boolean    
        
    'Setting the determination of a dot or a comma to FALSE until it was successfull.
    bDotOrCommaIncluded = FALSE
    
    if IsMissing(sDummy$) then
        '/// Opening a new document depending on <i>gApplication</i> value and closing it at the end.
        Call hNewDocument
        '/// Tools / Options / (Modul: gApplication) / General tabpage.
        ToolsOptions            
        '///+ <ol><li>Reading the string of the tabulator numeric field</li> 
        select case UCase(gApplication)
            case "WRITER",  "TEXTDOKUMENT"    : Call hToolsOptions("TEXTDOCUMENT","GENERAL")
                                                sCheckForSeparator = Tabulatorenabstand.GetText
            case "CALC", "TABELLENDOKUMENT"   : Call hToolsOptions("SPREADSHEET","GENERAL")
                                                sCheckForSeparator = Tabulator.GetText
            case "IMPRESS", "PRAESENTATION"   : Call hToolsOptions("PRESENTATION","GENERAL")
                                                sCheckForSeparator = Tabulatorenabstand.GetText                  
            case "DRAW", "ZEICHNUNG"          : Call hToolsOptions("DRAWING","GENERAL")
                                                sCheckForSeparator = Tabulatorenabstand.GetText
            case "GLOBALDOC", "GLOBALDOKUMENT": Call hToolsOptions("TEXTDOCUMENT","GENERAL")
                                                sCheckForSeparator = Tabulatorenabstand.GetText   
            case "HTML", "HTMLDOKUMENT"       : Call hToolsOptions("TEXTDOCUMENT","GENERAL")
                                                sCheckForSeparator = Tabulatorenabstand.GetText
            case else                         : warnlog cWhereIsThisFunction & "For this module ("& gApplication &") no decimal seperator setting exists."
        end select               
        Kontext "ExtrasOptionenDlg"
        ExtrasOptionenDlg.OK       
        if Instr(sCheckForSeparator, ",") > 0 then
            GetDecimalSeperator = ","
            bDotOrCommaIncluded = TRUE
        endif        
        if Instr(sCheckForSeparator, ".") > 0 then
            GetDecimalSeperator = "."
            bDotOrCommaIncluded = TRUE
        endif        
        Call hCloseDocument
    else            
        '///+ <li>or determining the seperator depending on the OPTIONAL value (string).</li></ol>
        'Get position of fraction mark / get IT
        if InStr (sDummy$, ",") > 0 then 
            GetDecimalSeperator = ","
            bDotOrCommaIncluded = TRUE    
        endif
        if InStr (sDummy$, ".") > 0 then 
            GetDecimalSeperator = "."
            bDotOrCommaIncluded = TRUE
        endif
    endif
    
    '/// If the determination failed the dot will be used (default) as decimal seperator.    
    if bDotOrCommaIncluded = FALSE then
        warnlog cWhereIsThisFunction & "Unable to determine decimal separator. Setting dot (.) as default."
        GetDecimalSeperator = "."                
    endif
    printlog "Info: Decimal Seperator is a '" & GetDecimalSeperator & "'." 
end function

sub sResetTheOffice as boolean
    Dim uno
    Dim ap
    Dim xViewRoot
    Dim apara(1) As new com.sun.star.beans.PropertyValue
    Dim temp()
    Dim i,x as integer
    Dim sString as string
    Dim fDeleteList(32000) as string
    Dim sLanguage as string
    Dim bError as boolean
    Dim sDefaultLocale as string
    Dim sDefaultLocaleCJK as string
    Dim sDefaultLocaleCTL as string
    Dim sfgetL10Nvalue as string    
    Dim SetupXML as String
    Dim SetupXMLNet as string
    Dim SetupXMLDefault as string
    Dim sLanOutIni as string

        sString = "qa:qatesttool:calc:options:inc:coption1.inc:: "
        sResetTheOffice = TRUE
        
    ' only run on UNIX platforms; there is a problem with the quickstarter on win32
    if ("unx" = gPlatgroup) then
        try
            SetupXML = gOfficePath & ConvertPath("user\registry\data\org\openoffice\Setup.xcu") 
            ' function 'fgetL10Nvalue' is also in this library
            sLanOutIni = fgetL10Nvalue(SetupXML)
        catch
            try
                ' BugID 98315 -> looking in networkpath for the language until bug will be fixed.
                    SetupXMLNet = gNetzOfficePath & ConvertPath("share\registry\data\org\openoffice\Setup.xcu")
                    sLanOutIni = fgetL10Nvalue(SetupXMLNet)            
            catch
                try
                    ' It is an English FAT version 645m9s2 or higher.                
                        SetupXMLDefault = gOfficePath & ConvertPath("share\registry\data\org\openoffice\Setup.xcu")
                        sLanOutIni = fgetL10Nvalue(SetupXMLDefault)
                catch
                        warnlog sString & SetupXML & " not found => can't get the correct Office-Language!."
                        sResetTheOffice = FALSE
                        Exit sub
                endcatch
            endcatch
        endcatch
            
        uno=hGetUnoService()
        
        'Get UI language
        try
            ap=uno.createInstance("com.sun.star.configuration.ConfigurationProvider")
            apara(0).Name="nodepath"
            apara(0).Value="/org.openoffice.Office.Linguistic/General"
            apara(1).Name="lazywrite"
            apara(1).Value=False
            xViewRoot=ap.createInstanceWithArguments("com.sun.star.configuration.ConfigurationUpdateAccess",aPara()) 
            sLanguage = sfgetL10Nvalue 
            sDefaultLocale = xViewRoot.getPropertyValue("DefaultLocale")
            sDefaultLocaleCJK = xViewRoot.getPropertyValue("DefaultLocale_CJK")
            sDefaultLocaleCTL = xViewRoot.getPropertyValue("DefaultLocale_CTL")
            printlog "Old UI language: '" + sLanOutIni + "'"
            printlog "Old default locale: '" + sDefaultLocale + "'"
            printlog "Old default locale CJK: '" + sDefaultLocaleCJK + "'"
            printlog "Old default locale CTL: '" + sDefaultLocaleCTL + "'"
            xViewRoot.dispose()
            bError = FALSE
        catch
            warnlog sString + "Failed to read UI language."
            bError = TRUE
        endcatch
    
        if NOT bError then
            'Close OOo
            try
                ' To prevent restarting of OOo, the try/catch is around this and
                ' to prevent messages about communication errors
                printlog ResetApplication
                FileExit "SynchronMode", TRUE
                try
                    ' It is no error, if this fails - so it gets its own try/catch
                    kontext
                    if active.exists(5) then
                        active.no 'discard changes
                    endif
                catch
                endcatch
                bError = FALSE
            catch
                warnlog sString + "Failed to close OOo."
                bError = TRUE
            endcatch
            sleep 10 'To wait until OOo is realy away
        endif
    
        'only act, if no error and if language <> ''
         if (NOT bError AND sLanguage <> "") then
             'Remove user directory
            try
                if (right(gOfficePath,1)=gPathSigne) then
                    'Dir doesn't work, is a path singe is at the end
                    gOfficePath = left(gOfficePath,len(gOfficePath)-1)
                endif
                printlog "Going to delete directory: '" + gOfficePath + "'"
                if (dir(gOfficePath) = "") then
                    qaErrorlog "Directory is already deleted."
                else
                    rmDir (gOfficePath)
                    if (dir(gOfficePath) <> "") then
                        warnlog "Directory wasn't deleted."
                    endif
                endif
                bError = FALSE
            catch
                warnlog sString + "Failed to delete user directory."
                bError = TRUE
            endcatch
        endif
    
        'Start OOo and restore language
        'Needs only to be done, if UI language wasn't the default (!= "")
        if ((sLanguage & sDefaultLocale & sDefaultLocaleCJK & sDefaultLocaleCTL) <> "") then
            try
                hStartTheOffice
                Call hDisableQuickstarter
                'Here we need the Exit from a running Quickstarter...            
                Call ExitRestartTheOffice
                uno=hGetUnoService()
                ap=uno.createInstance("com.sun.star.configuration.ConfigurationProvider")
                apara(0).Name="nodepath"
                apara(0).Value="/org.openoffice.Office.Linguistic/General"
                apara(1).Name="lazywrite"
                apara(1).Value=False
                xViewRoot=ap.createInstanceWithArguments("com.sun.star.configuration.ConfigurationUpdateAccess",aPara())
                if (sLanOutIni <> "") then                
                    printlog "Old UI language: '" + sLanOutIni + "'"
                    xViewRoot.setPropertyValue("UILocale", sLanOutIni)
                    xViewRoot.commitChanges()
                endif
                if (sDefaultLocale <> "") then
                    printlog "Old default locale: '" + sDefaultLocale + "'"
                    xViewRoot.setPropertyValue("DefaultLocale", sDefaultLocale)
                    xViewRoot.commitChanges()
                endif
                if (sDefaultLocaleCJK <> "") then
                    printlog "Old default locale CJK: '" + sDefaultLocaleCJK + "'"
                    xViewRoot.setPropertyValue("DefaultLocale_CJK", sDefaultLocaleCJK)
                    xViewRoot.commitChanges()
                endif
                if (sDefaultLocaleCTL <> "") then
                    printlog "Old default locale CTL: '" + sDefaultLocaleCTL + "'"
                    xViewRoot.setPropertyValue("DefaultLocale_CTL", sDefaultLocaleCTL)
                    xViewRoot.commitChanges()
                endif
                if xViewRoot.hasPendingChanges() then
                    warnlog(sFileFunction+"Changes still pending...")
                endif
                xViewRoot.dispose()            
            catch
                warnlog sString + "Failed to set UI language."
                exit sub
            endcatch
        endif
        
        Call ExitRestartTheOffice
    endif
end sub

'-------------------------------------------------------------------------

sub raiseApplication
' Try to solve focus problem on MacOS X; After calling this function, OOo should be most front;
    Dim i as integer
    Dim a as integer
    Dim b as integer
    Dim tBundle as string
    Dim aPath

    ' Calling just the .app with open on MacOS X via shell command
    if gPlatform = lcase("osx") then
        aPath = split(gNetzOfficePath, gPathSigne)
        a=0
        ' make sure 'Contents' is just one time in path
        for i=0 to uBound(aPath)
            if "Contents" = aPath(i) then
                a=a+1
            endif
        next i
        ' exit if not
        if a<>1 then
             exit sub
        end if
        i=inStr(gNetzOfficePath, "Contents")
        tBundle=left(gNetzOfficePath, i-2)
        shell("open",1 ,tBundle, true)
    end if
end sub

