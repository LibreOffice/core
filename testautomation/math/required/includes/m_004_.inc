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
'* Owner : thorsten.bosbach@oracle.com
'*
'* short description :
'*
'\*****************************************************************

sub M_004_
   Printlog Chr(13)+"--  Format Menu  m_004_  --"

   Call tFormatFonts
   Call tFormatFontsize
   Call tFormatDistance
   Call tFormatAlignment
   Call tFormatTextmode
end sub

testcase tFormatFonts
   Dim i as integer
   dim iCount as integer
   '/// open application ///'
   Call hNewDocument
   sleep (5)
   '/// Format->Fonts ///'
   FormatFonts
   Kontext "Schriftarten"
   Call DialogTest (Schriftarten)
   '/// select every entry from 'Modify' and do ///'
   Aendern.OpenMenu
   iCount = hMenuItemGetCount
   if (iCount <> 7) then
       warnlog "There are Entries missing; expected: '7'; found: '" + iCount +"'"
   endif
   for i=1 to iCount
      sleep 2
      printlog "  Font for Menuitem Nr. " & i
 	   sleep 2
      hMenuSelectNr ( i )
      Sleep 2
      Kontext "Schriften"
         Call DialogTest (Schriften)
         if (i<5) then
             try
                 '/// check checkbox 'bold' ///'
                 fett.check
                 '/// check checkbox 'italic' ///'
                 kursiv.check
             catch
                 warnlog "fett and/or kursiv checkbox doesn't exist"
             endcatch
         else
             if (fett.isvisible OR kursiv.isVisible) then
                 warnlog "fett and/or kursiv checkbox exists"
             endif
         endif
         sleep 1
         '/// cancel dialog 'fonts' ///'
      Schriften.Cancel
      Sleep 2
      Kontext "Schriftarten"
      Aendern.OpenMenu
   next i
   Kontext "Schriftarten"
   hMenuClose
   '/// cklick button 'default' ///'
   Standard.Click
   Kontext "Active"
   if active.exists (5) then
      Active.No ' should changes ybe saved as default (yes/no)
   else
      warnlog "should changes be saved as default (yes/no) failed :-("
   endif
   Kontext "Schriftarten"
   '/// cancel dialog 'fonts' ///'
   Schriftarten.Cancel
   '/// close document ///'
   Call hCloseDocument
endcase

testcase tFormatFontsize
   '/// open application ///'
   Call hNewDocument
   '/// Format->FontSize ///'
   FormatFontSize
   Kontext "Schriftgroessen"
      Call DialogTest (Schriftgroessen)
      '/// press button MORE on 'Base size' ///'
      BasisGroesse.more
      '/// press button MORE on 'Text' ///'
      TextGroesse.more
      '/// press button MORE on 'Indexes' ///'
      Indizes.more
      '/// press button MORE on 'Functions' ///'
      Funktionen.more
      '/// press button MORE on 'Operators' ///'
      Operatoren.more
      '/// press button MORE on 'Limits' ///'
      Grenzen.more
      sleep 1
      '/// press button 'default' ///'
      Standard.Click
      kontext "active"
      if active.exists (5) then
         Active.No ' should changes ybe saved as default (yes/no)
      else
         warnlog "should changes be saved as default (yes/no) failed :-("
      endif
      Kontext "Schriftgroessen"
   '/// cancel dialog 'Font Sizes' ///'
   Schriftgroessen.Cancel
   sleep 2
'/// close document ///'
   Call hCloseDocument
endcase

testcase tFormatDistance
   Dim i as integer
   '/// open application ///'
   Call hNewDocument
   '/// Format->Spacing ///'
   FormatSpacing
   Kontext "Spacing"
      Call DialogTest (Spacing)
      Category.Open
      sleep 1
      for i=1 to hMenuItemGetCount
         hMenuSelectNr (i)
         Sleep 1
         Call DialogTest (Spacing,i)
         Select Case i
            Case 1
               '/// select 'Spacing' from Category menu ///'
               '///+ click MORE button on '' ///'
               SpacingSpacing.More
               '///+ click MORE button on '' ///'
               LineSpacing.More
               '///+ click MORE button on '' ///'
               RootSpacing.More
            Case 2
               '/// select 'Indexing' from Category menu ///'
               '///+ click MORE button on '' ///'
               Superscript.More
               '///+ click MORE button on '' ///'
               Subscript.More
            Case 3
               '/// select 'Fractions' from Category menu ///'
               '///+ click MORE button on '' ///'
               Numerator.More
               '///+ click MORE button on '' ///'
               Denominator.More
            Case 4
               '/// select 'Fraction bars' from Category menu ///'
               '///+ click MORE button on '' ///'
               ExcessLength.More
               '///+ click MORE button on '' ///'
               Weight.More
            Case 5
               '/// select 'Limits' from Category menu ///'
               '///+ click MORE button on '' ///'
               UpperLimit.More
               '///+ click MORE button on '' ///'
               LowerLimit.More
            Case 6
               '/// select 'Brackets' from Category menu ///'
               '///+ click MORE button on '' ///'
               ExcessSize.More
               '///+ click MORE button on '' ///'
               BracketSpacing.More
               ScaleAllBrackets.Check
               '///+ click MORE button on '' ///'
                  ExcessSizeAll.More
            Case 7
               '/// select 'Matrixes' from Category menu ///'
               '///+ click MORE button on '' ///'
               MatrixLineSpacing.More
               '///+ click MORE button on '' ///'
               ColumnSpacing.More
            Case 8
               '/// select 'Symbols' from Category menu ///'
               '///+ click MORE button on '' ///'
               PrimaryHeight.More
               '///+ click MORE button on '' ///'
               MinimunSpacing.More
            Case 9
               '/// select 'Operators' from Category menu ///'
               '///+ click MORE button on '' ///'
               OperatorsExcessSize.More
               '///+ click MORE button on '' ///'
               OperatorsSpacing.More
            Case 10
               '/// select 'Borders' from Category menu ///'
               '///+ click MORE button on '' ///'
               BordersLeft.More
               '///+ click MORE button on '' ///'
               BordersRight.More
               '///+ click MORE button on '' ///'
               BordersTop.More
               '///+ click MORE button on '' ///'
               BordersBottom.More
            Case Else
               warnlog "More items than expected :-("
         End Select
         Category.Click
      next i
      hMenuClose
      Sleep 1
      Default.Click
      kontext "active"
      if active.exists (5) then
         Active.No ' should changes ybe saved as default (yes/no)
      else
         warnlog "should changes be saved as default (yes/no) failed :-("
      endif
      Kontext "Spacing"
   '/// cancel dialog 'Spacing' ///'
   Spacing.Cancel
   Sleep 2
'/// close document ///'
   Call hCloseDocument
endcase

testcase tFormatAlignment
   '/// open application ///'
   Call hNewDocument
   '/// Format->Alignment ///'
   FormatAlignment
   Kontext "Ausrichtung"
   Call DialogTest (Ausrichtung)
      '/// check Radiobutton 'Left' ///'
      Links.Check
      '/// check Radiobutton 'Centered' ///'
      Zentriert.Check
      '/// check Radiobutton 'Right' ///'
      Rechts.Check
      '/// check Button 'Default' ///'
      Standard.Click
      kontext "active"
      if active.exists (5) then
         Active.No ' should changes ybe saved as default (yes/no)
      else
         warnlog "should changes be saved as default (yes/no) failed :-("
      endif
   Kontext "Ausrichtung"
   Ausrichtung.Cancel
   '/// close document ///'
   Call hCloseDocument
endcase

testcase tFormatTextmode
'/// open application ///'
   Call hNewDocument
   '/// Format->Text Mode ///'
   FormatTextMode
   sleep 1
   '/// Format->Text Mode ///'
   FormatTextMode
'/// close document ///'
   Call hCloseDocument
endcase

