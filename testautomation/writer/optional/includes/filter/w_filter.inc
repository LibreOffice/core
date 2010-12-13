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
'* Owner : helge.delfs@oracle.com
'*
'* short description :
'*
'*******************************************************************
' #1 tSammleImportFilter
' #1 tCollectSaveAsFilter
' #1 tCollectExportFilters
' #1 fReadComparePrintLists
' #1 tGroupNameCheck
'\******************************************************************

testcase tSammleImportFilter
'/// compare IST- with SOLL- import filter state ///'

 Dim i as Integer
 Dim j as Integer
 Dim x as Integer
 Dim Ende as Integer
 Dim EndeSoll as Integer

' Dim Zaehler as Integer               ' count entries with "(STAROFFICE WRITER)" in IST
 Dim gZaehler  as Integer             ' do the same as 'i' in 'compare for-next' ?
 Dim gZaehler2  as Integer             ' do the same as 'i' in 'compare for-next' ?

 Dim lsListe(350) as String           ' IST list gets generated with tested SO and written into
 Dim Eintrag as String                ' during compare: entry (i) from IST list
 Dim lsListeFail(350) as String       ' List that gets set from the compare function, if anny filters are not available, optional parammeter to function

'----new implementation of filter lists
 Dim sPlatform                        as String
 Dim fStandardFilterList              as String
 Dim fStandardLanguageFilterList      as String
 Dim fExceptionLanguageFilterList     as String
 Dim fCompleteDiffFilterList          as String
 Dim fW4WDiffFilterList               as String
 Dim fActualFilterFile                as String
 Dim fSortedFilterFile                as String
 Dim iFilterCount                     as Integer
 Dim a as integer
 Dim b as integer
 Dim c as integer
'--------------------------------------

' INIT Stuff
   gZaehler = 0
   lsListe(0)=0

'/// There are 3 files with filter lists ///'
'///+ 1. Language independent, System independent, Standard installation                 : if_st_all.txt     ///'
'///+ 2. Language   dependent, System independent, Standard installation                 : if_st_xx_all.txt ///'
'///+ 2. Language   dependent, System independent, Standard installation, Exceptions     : if_st_xx_exe.txt ///'
'///+ 3. Language independent, System dependent, Complete installation diff to 1.) & 2.) : if_cod_xxx.txt  ///'
'///+ 3. Language independent, System dependent, Standard installation diff to 1.)       : if_w4w_xxx.txt  ///'
'///+ they are all found in "input\\writer\\la_sp\\" ///'
'/// System depends on: w95, sol, lin; if system is not regcognizeable: Fallback to lin ///'
'///+ Path to filter lists: 'input\\writer\\la_sp\\' ///
'///+ Filter files have to be saved as Unicode UTF8 encoded ///'

' since utf8 encoding isn't supported as list-write-format it doesn't make sense to save the list

   ' set Platform variable
   'printlog "  + + +  gPCName: "+ gPCName+ " gPlatgroup: "+ gPlatgroup +" gPlatform: "+ gPlatform + " gSYSName: "+gSYSName
   select case gPlatgroup
      case "w95"   : sPlatform = "w95"
      case "unx"   :
         select case gPlatform
            case "lin"   : sPlatform = "lin"
            case "x86"   : sPlatform = "lin" ' i think there were w4w licensing problems
            case "sol"   : sPlatform = "w95"
            case else : sPlatform = "lin"  ' minimal filter list
         end select
      case else : sPlatform = "lin"  ' minimal filter list
   end select

   'this one gets hard coded!
   fStandardFilterList = ConvertPath(gTesttoolPath + "writer\optional\input\filter\if_st_all." + "txt")
   fStandardLanguageFilterList = ConvertPath(gTesttoolPath + "writer\optional\input\filter\if_st_"+ iSprache+"_all." + "txt")
   fExceptionLanguageFilterList = ConvertPath(gTesttoolPath + "writer\optional\input\filter\if_st_"+ iSprache+"_exe." + "txt")

   fCompleteDiffFilterList = ConvertPath(gTesttoolPath + "writer\optional\input\filter\if_cod_" + sPlatform + ".txt")
   fW4WDiffFilterList = ConvertPath(gTesttoolPath + "writer\optional\input\filter\if_w4w_" + sPlatform + ".txt")

   fActualFilterFile = ConvertPath(gTesttoolPath + "writer\optional\input\filter\actual.txt") 'unsorted
   fSortedFilterFile = ConvertPath(gTesttoolPath + "writer\optional\input\filter\sorted.txt") 'sorted

'' generate IST-list and write to file ///'
   FileOpen
   sleep (3)
   Kontext "OeffnenDlg"
   sleep (3)
   iFilterCount = Dateityp.GetItemCount ' have to substract already used ones at the end somewhere...
   sleep (3)
   for i=1 to Dateityp.GetItemCount
      ListAppend ( lsListe(), DateiTyp.GetItemText (i) )
   next i
   OeffnenDlg.Cancel
   'ListWrite(lsListe(),fActualFilterFile, "utf8")
   'Call ListCopy(lsListe(),lsListeSort())
   Call ListSort(lsListe())
   'ListWrite(lsListe(),fSortedFilterFile, "utf8")
   '   printlog "-------------------------------------------------------------"
   '         for i=1 to ListCount ( lsListeSort() )
   '             printlog lsListeSort(i)
   '         next i
   '   printlog "-------------------------------------------------------------"
   'Printlog "- Got Filters from Office; written to: " + fActualFilterFile + "; Count: "+ ListCount (lsListe())
   'Printlog "- Got Filters from Office; written to: " + fSortedFilterFile + chr(13)

   lsListeFail(0) = 0
   printlog "'///- comparing ## Language independent ## Standard installation ## filters with existing ones"
   printlog "----- found : "+ fReadComparePrintLists (fStandardFilterList, lsListe(),lsListeFail()) + chr(13)

   a = ListCount (lsListeFail()) ' not found filters
   if (a > 0) then ' sort out filters for platforms, where we don't have licenses for OR the translator has good reasons....(swedish)
      if (iSprache = 46) then
         b = fReadComparePrintLists (fExceptionLanguageFilterList, lsListeFail())
         printlog "----- found : "+ b + chr(13)
         if (ListCount (lsListeFail()) <> 0) then
            a = ListCount ( lsListeFail() )
            for i=1 to a
               Printlog lsListeFail(i)
            next i
            printlog "----- NOT found : "+ c + chr(13)
         else
            qaErrorLog "--------- EVERYTHING is fine now, ignore every failure above about not found filters! :-) --------------"
         end if
      end if
      if (sPlatform = "lin") then
         qaErrorLog "ok there are failures, lets check them: sort out filters that are not allowed on this platform! (LINUX) :-)"
         b = fReadComparePrintLists (fW4WDiffFilterList, lsListeFail())
         printlog "----- found : "+ b + chr(13)
         if (ListCount (lsListeFail()) <> 0) then
            warnlog "By now, there are STILL some Filters realy missing :-("
            c = ListCount ( lsListeFail() )
            for i=1 to c
               Printlog lsListeFail(i)
            next i
            printlog "----- NOT found : "+ c + chr(13)
         else
            qaErrorLog "--------- EVERYTHING is fine now, ignore every failure above about not found filters! :-) --------------"
         end if
      else
         Warnlog ("The above filters are missing: " + a)
      end if
   end if

   printlog "'///- comparing ## Language   dependent ## Standard installation ## filters with existing ones"
   b = fReadComparePrintLists (fStandardLanguageFilterList, lsListe())
   printlog "----- found : "+ b + chr(13)
   if b = 0 then
      warnlog "         No filters found :-( - maybe because list-file is empty, please create it!"
      warnlog "         take the file </testtool/input/writer/la_sp/if_st_1.all> and put all filters for this language on the same position into the file: " +fStandardLanguageFilterList
   end if
   a = ListCount (lsListeFail()) ' not found filters
   if (a > 0) then
      Warnlog ("The above filters are missing: " + a)
   end if

    '///' delete ------------ filters from IST-list (used filters appear twice) ///'
    '/// <font color=#ff0000> TBO TODO: select them! nice behaviour i think :-) </FONT> ///'
         printlog "- remove seperators ----------------------------"
         gZaehler2 = 0
         for i=1 to ListCount ( lsListe() )
            Eintrag = lsListe(i)
            if instr(Eintrag,"---------") then
             gZaehler2 = gZaehler2 + 1
             ListDelete ( lsListe(), i )               ' delete seperator
             i=i-1                                     ' so the i-counter has to be decremented by one
            end if
         next i
         printlog "   + (" + gZaehler2 + "/8) seperators are available."
         if gZaehler2 <> 8 then
            warnlog " there are seperators missing or too much :-("
            Printlog " check if all groups are available! the groupnames can be found in the tostcase <tGroupNameCheck> later on"
            warnlog " !!! NO FILTER IS ALLOWED TO HAVE THE SAME NAME AS ANY GROUPNAME !!! because than this group won't show up in the FileOpen Dialog!"
         end if
   '///' delete <...> filters from IST-list (used filters appear twice) ///'
         printlog "- Already used filters in previous tests."
         gZaehler = 0
         for i=1 to ListCount ( lsListe() )
            Eintrag = lsListe(i)
            if Left$(Eintrag,1) = "<" and Right$(Eintrag,1) = ">" then
             gZaehler = gZaehler + 1
              printlog "" + Eintrag
             ListDelete ( lsListe(), i )               ' delete already used filter (i)
             i=i-1                                     ' so the i-counter has to be decremented by one
            end if
         next i
         printlog "   + " + gZaehler + " Filters have been used in previous tests."
         printlog "++++++ this was now : " + (iFilterCount - gZaehler - gZaehler2) +" existing filters in office"

   if ListCount (lsListe()) > 50 then
      printlog chr(13)+"'/// - this is a COMPLETE Installation ?! (>50 filters still available): "+ListCount (lsListe()) + chr(13)
      printlog "'///- comparing  ## Language independent ## Complete installation ## filters with existing ones"
      printlog "----- found : "+ fReadComparePrintLists (fCompleteDiffFilterList, lsListe()) + chr(13)
   else
      printlog chr(13)+"'/// - this is a STANDARD Installation ?! (<50 filters still available): "+ListCount (lsListe()) + chr(13)
   end if

   if ListCount ( lsListe() ) <> 0 then
      printlog Chr(13)
      Warnlog "*** The following Filters are available, but nobody wants them??'!" + "; Count: "+ ListCount (lsListe())
      Warnlog ("please compare them to the missing ones from above!")
      for i=1 to ListCount ( lsListe() )
         Printlog lsListe(i)
      next i
   end if
endcase

'------------------------------------------------------------------------------------------------------------------------------

testcase tCollectSaveAsFilter
 Dim i as Integer
 Dim j as Integer
 Dim x as Integer

 Dim lsListe(350)      as String           ' IST list gets generated with tested SO and written into
 Dim iFilterCount      as Integer
 Dim fActualFilterFile as String
 Dim fSortedFilterFile as String

   ListAllDelete(lsListe())
   x = 0

   fActualFilterFile = ConvertPath(gOfficePath + "user/work/sa_actua."+iSprache) 'unsorted
   fSortedFilterFile = ConvertPath(gOfficePath + "user/work/sa_sorte."+iSprache) 'sorted

'/// go into every application and print out the filters found under 'File -> SaveAs' ///'
   for j = 1 to 7
      select case j
         case 1: gApplication = "WRITER"
         case 2: gApplication = "CALC"
         case 3: gApplication = "IMPRESS"
         case 4: gApplication = "DRAW"
         case 5: gApplication = "MASTERDOCUMENT"
         case 6: gApplication = "MATH"
         case 7: gApplication = "HTML"
      end select
      hNewDocument()
   '' generate IST-list and write to file ///'
      FileSaveAs
      sleep (3)
      Kontext "SpeichernDlg"
      sleep (3)
      iFilterCount = Dateityp.GetItemCount ' have to substract already used ones at the end somewhere...
      x = x + iFilterCount
      sleep (3)
      printlog " ---- " + gApplication + " : " + iFilterCount
      for i=1 to Dateityp.GetItemCount
         ListAppend ( lsListe(), DateiTyp.GetItemText (i) )
         Printlog DateiTyp.GetItemText (i)
      next i
      Printlog "-----------------------------------------------"
      SpeichernDlg.Cancel
      hCloseDocument()
   next j

   Printlog chr(13)+" This are "+ x + " - from 85 expected." + chr(13)
   if x <> 85 then warnlog "Somewhere Over the rainbow... there are filters missing / too much :-("

   if (app.dir(fActualFilterFile) = "") then
      Call ListWrite(lsListe(),fActualFilterFile, "utf8")
      Printlog "- Got Filters from Office; written to: " + fActualFilterFile + "; Count: "+ ListCount (lsListe())
   end if
   if (app.dir(fSortedFilterFile) = "") then
      Call ListSort(lsListe())
      Call ListWrite(lsListe(),fSortedFilterFile, "utf8")
      Printlog "- Got Filters from Office; written to: " + fSortedFilterFile + chr(13)
   end if
endcase

'------------------------------------------------------------------------------------------------------------------------------

testcase tCollectExportFilters
 Dim i as Integer
 Dim j as Integer
 Dim x as Integer

 Dim lsListe(350)      as String           ' IST list gets generated with tested SO and written into
 Dim iFilterCount      as Integer
 Dim fActualFilterFile as String
 Dim fSortedFilterFile as String

   ListAllDelete(lsListe())
   x = 0

   fActualFilterFile = ConvertPath(gOfficePath + "user/work/ex_actua."+iSprache) 'unsorted
   fSortedFilterFile = ConvertPath(gOfficePath + "user/work/ex_sorte."+iSprache) 'sorted

'/// go into every application and print out the filters found under 'File -> SaveAs' ///'
   for j = 1 to 7
      select case j
         case 1: gApplication = "WRITER"
         case 2: gApplication = "CALC"
         case 3: gApplication = "IMPRESS"
         case 4: gApplication = "DRAW"
         case 5: gApplication = "MASTERDOCUMENT"
         case 6: gApplication = "MATH"
         case 7: gApplication = "HTML"
      end select
      hNewDocument()
   '' generate IST-list and write to file ///'
      FileExport
      sleep (3)
      Kontext "SpeichernDlg"
      sleep (3)
      iFilterCount = Dateityp.GetItemCount ' have to substract already used ones at the end somewhere...
      x = x + iFilterCount
      sleep (3)
      printlog " ---- " + gApplication + " : " + iFilterCount
      for i=1 to Dateityp.GetItemCount
         ListAppend ( lsListe(), DateiTyp.GetItemText (i) )
         Printlog DateiTyp.GetItemText (i)
      next i
      Printlog "-----------------------------------------------"
      SpeichernDlg.Cancel
      hCloseDocument()
   next j

    Printlog chr(13)+" This are "+ x + " - from 65 expected." + chr(13)
    if x <> 65 then warnlog "Somewhere Over the rainbow... there are filters missing / too much :-("

   if (app.dir(fActualFilterFile) = "") then
      Call ListWrite(lsListe(),fActualFilterFile, "utf8")
      Printlog "- Got Filters from Office; written to: " + fActualFilterFile + "; Count: "+ ListCount (lsListe())
   end if
   if (app.dir(fSortedFilterFile) = "") then
      Call ListSort(lsListe())
      Call ListWrite(lsListe(),fSortedFilterFile, "utf8")
      Printlog "- Got Filters from Office; written to: " + fSortedFilterFile + chr(13)
   end if
' Should look like this:
' ---- WRITER : 3
'PDF - Portable Document Format (.pdf)
'------------------------------------------
'XHTML (.xhtml)
'-----------------------------------------------
' ---- CALC : 3
'PDF - Portable Document Format (.pdf)
'------------------------------------------
'XHTML (.xhtml)
'-----------------------------------------------
' ---- IMPRESS : 23
'HTML Document (StarOffice Impress) (.html;.htm)
'PDF - Portable Document Format (.pdf)
'Macromedia Flash (SWF) (.swf)
'------------------------------------------
'BMP - Windows Bitmap (.bmp)
'EMF - Enhanced Metafile (.emf)
'EPS - Encapsulated PostScript (.eps)
'GIF - Graphics Interchange Format (.gif)
'JPEG - Joint Photographic Experts Group (.jpg;.jpeg;.jfif;.jif;.jpe)
'MET - OS/2 Metafile (.met)
'PBM - Portable Bitmap (.pbm)
'PCT - Mac Pict (.pct)
'PGM - Portable Graymap (.pgm)
'PNG - Portable Network Graphic (.png)
'PPM - Portable Pixelmap (.ppm)
'PWP - PlaceWare (.pwp)
'RAS - Sun Raster Image (.ras)
'SVG - Scalable Vector Graphics (.svg)
'SVM - StarView Metafile (.svm)
'TIFF - Tagged Image File Format (.tif;.tiff)
'WMF - Windows Metafile (.wmf)
'XPM - X PixMap (.xpm)
'XHTML (.xhtml)
'-----------------------------------------------
' ---- DRAW : 22
'HTML Document (StarOffice Draw) (.html;.htm)
'PDF - Portable Document Format (.pdf)
'Macromedia Flash (SWF) (.swf)
'------------------------------------------
'BMP - Windows Bitmap (.bmp)
'EMF - Enhanced Metafile (.emf)
'EPS - Encapsulated PostScript (.eps)
'GIF - Graphics Interchange Format (.gif)
'JPEG - Joint Photographic Experts Group (.jpg;.jpeg;.jfif;.jif;.jpe)
'MET - OS/2 Metafile (.met)
'PBM - Portable Bitmap (.pbm)
'PCT - Mac Pict (.pct)
'PGM - Portable Graymap (.pgm)
'PNG - Portable Network Graphic (.png)
'PPM - Portable Pixelmap (.ppm)
'RAS - Sun Raster Image (.ras)
'SVG - Scalable Vector Graphics (.svg)
'SVM - StarView Metafile (.svm)
'TIFF - Tagged Image File Format (.tif;.tiff)
'WMF - Windows Metafile (.wmf)
'XPM - X PixMap (.xpm)
'XHTML (.xhtml)
'-----------------------------------------------
' ---- GLOBALDOC : 7
'PDF - Portable Document Format (.pdf)
'------------------------------------------
'StarOffice 6.0 Text Document  (.sxw)
'StarWriter 5.0  (.sdw)
'StarWriter 4.0  (.sdw)
'StarWriter 3.0  (.sdw)
'Open Office Text (.oot)
'-----------------------------------------------
' ---- MATH : 1
'PDF - Portable Document Format (.pdf)
'-----------------------------------------------
' ---- HTML : 7
'PDF - Portable Document Format (.pdf)
'------------------------------------------
'StarOffice Text Document (StarOffice Writer/Web) (.sxw)
'StarWriter 5.0 (StarOffice Writer/Web) (.sdw)
'StarWriter 4.0 (StarOffice Writer/Web) (.sdw)
'StarWriter 3.0 (StarOffice Writer/Web) (.sdw)
'Open Office Text (StarOffice Writer/Web) (.oot)
'-----------------------------------------------
endcase

function fReadComparePrintLists (fFilterList as string, gFilterList() as string, optional gFilterListFail() as string) as integer
'/// input :  filename of the to load list, existing list, return filter list of fails ///'
'///+ output:  changed existing list, generates file of list, if not available.. sometimes ///'
'///+          count of found list entries  ///'
'///+          optional gFilterListFail gets set to the missing filters, for further evaluation! ///
'///+ function: loads list from file, searches, prints, deletes found listentries from given list ///'
'///+            prints listentries from loaded list, which were not found ///'

 Dim lFilterList(350)     as String
 Dim Eintrag     as String
 Dim i as Integer
 Dim j as Integer
 Dim x as Integer
 Dim Ende as Integer
 Dim EndeSoll as Integer
 Dim gZaehler as Integer             ' do the same as 'i' in 'compare for-next' ?

'----temp stuff only used, if filternames change again :-[
 Dim lsTemp(350) as String
 dim iTemp as integer
 dim sTemp as string
 dim fTempFilterList as string
'--------------------------------------

 lFilterList(0) = 0
 fReadComparePrintLists = -1 ' worst case

'/// read Standard-list ONE from file ///'
   if Dir ( fFilterList ) <> "" then
      ListRead ( lFilterList(), fFilterList, "utf8" )
      Printlog "  - Read Filters-to-find-List from: " + fFilterList + "; Count: "+ ListCount (lFilterList())
      '------------temp stuff------------- to get the part in braces '()'
      '   lsTemp(0) = 0
      '   for iTemp =1 to ListCount (lFilterList())
      '      sTemp = lFilterList(iTemp)
      '      ListAppend (lsTemp(), mid (sTemp, inStr(sTemp, "("), len(stemp)+1-inStr(sTemp, "(") ))
      '      Printlog " " + lsTemp(iTemp)
      '   next iTemp
      '   fTempFilterList = ConvertPath(gTesttoolPath + "writer\optional\input\filter\if_st_XX_all_ext.txt")
      '   ListRead ( lsTemp(), fTempFilterList, "utf8" )
      '   for iTemp =1 to ListCount (lFilterList())
      '      sTemp = lFilterList(iTemp) + lsTemp(iTemp)
      '      Printlog " " + sTemp
      '      lsTemp(iTemp) = sTemp
      '   next iTemp
      '   ListWrite ( lsTemp(), fFilterList+"X", "utf8" )
      '-----------------------------------
'/// delete ONE filters from both, IS and ONE, lists and print out found & not found ones ///'
      Call ListSort(lFilterList())
      Call ListSort(gFilterList())
      printlog "  - List Filters which have been found in both Lists:"
      x=0 ' count found filters
      Ende = ListCount ( gFilterList() )
      for i=1 to Ende
          gZaehler = gZaehler + 1
          Eintrag = gFilterList(i)
          for j=1 to ListCount ( lFilterList() )
             if Eintrag = lFilterList(j) then          ' if entry found -> delete from IST & SOLL list
                Printlog "    - " + lFilterList(j)
                x=x+1
                ListDelete ( lFilterList(), j )
            ' don't modify the gFilterList, copy the not found entry to new global list !!!!
                ListDelete ( gFilterList(), i )
                i=i-1
         ' here i have to check the function, how a list gets deleted... ! i think evrything from that entry on moves one up
                j = 1000                               ' goto next entry
'                Fehler = FALSE                         ' -> found state
             else
'                Fehler = TRUE
             end if
          next j
          if gZaehler = Ende then i=1000       ' Abbruch, weil die Schleife einmal druchgegangen ist
      next i
      'printlog "+++ these are " + x
   else
      warnlog " Missing list file, create empty file: "+fFilterList
      ListWrite ( lFilterList(), fFilterList, "utf8")
   end if
   '/// print filters which were not found from ONE list ///'
   if ListCount ( lFilterList() ) <> 0 then
      qaErrorLog "The following filters aren't available in the Office! look what happend to them !!!" + "; Count: "+ ListCount (lFilterList())
      for i=1 to ListCount ( lFilterList() )
         Printlog "" + lFilterList(i)
      next i
      ListCopy (lFilterList(), gFilterListFail())
   else
      ListAllDelete (gFilterListFail())
   end if
   fReadComparePrintLists = x
end function

'------------------------------------------------------------------------------------------------------------------------------

testcase tGroupNameCheck
    if gPlatGroup = "unx" then
        Printlog "No testing under Unix/Linux. Exiting testcase."
        goto endsub
    end if

'///' inspired from #97305 : 86+88: missing import filters in fileopen dialog ///'
'///+' -> 97723 97758 ///'
'///' solution for sucha problem is, that the group name (Webseiten) is the same as a filtername (Webseite) in the translation. ///'
'///+' so you have to keep an eye on the names in 'UI.xml' & 'TypeDetection.xml' from '\share\config\registry\instance\org\openoffice\Office' ///'
'///+' they have to be different! ///'

  Dim saGroups (50) as string
  dim i as integer

'TypeDetection.xml
   saGroups (0)=0
' cfg:value; xml:lang = de...
'   saGroups = TBOXMLGetFirstCharsForElement ( "UI;FilterClassification;GlobalFilters;Classes;FilterClass;DisplayName;cfg:value", UIxml, TRUE )
   TBOXMLGetListOfFilterGroups (iSprache, saGroups ())
   for i = 1 to ListCount (saGroups())
      Printlog " "+i+": '" + saGroups(i)+"'"
   next i
endcase

function TBOXMLGetListOfFilterGroups ( iLang as integer, sList() as string  ) as String
'/// parse xml file for special needs ///'
  dim sElementLine as String
  dim sXMLFile as String

  Dim bCloseLocal as Boolean
  dim i as integer
  Dim x as integer
  Dim y as integer
  dim sLang, sClass as string
  dim iAttrCount as integer

   sElementLine = "UI;FilterClassification;GlobalFilters;Classes"
   sXMLFile = gNetzOfficePath & ConvertPath("Basis\share\registry\res\" + gISOLang + "\org\openoffice\Office\UI.xcu")

   if (dir(sXMLFile) = "") then
       qaErrorLog "File doesn't exist :-( '" + sXMLFile + "'"
       exit function
   end if
   SAXReadFile ( sXMLFile )
   bCloseLocal = TRUE
   call hXMLGotoElement ( sElementLine )

   for i = 1 to SAXGetChildCount
      SAXSeekElement ( i )
'      if SAXGetNodeType <> NodeTypeCharacter then
'         Printlog "             '"+SAXGetAttributeName (1) +"' : '"+SAXGetAttributeValue (1) +"' :"
         for y = 1 to SAXGetChildCount
            SAXSeekElement ( y )
            SAXSeekElement ( 1 )
            sLang = SAXGetAttributeValue (1)
            SAXSeekElement ( 1 )
            if (y = 1) then
'               if SAXGetNodeType = NodeTypeCharacter then Printlog " "+ConvertLanguage2( slang )+":"+slang +":"+i+":"+SAXGetChars
               if (ConvertLanguage2( slang, TRUE ) = iLang) then ListAppend (sList(),SAXGetChars)
            else
'               if SAXGetNodeType = NodeTypeCharacter then Printlog " "+":"+slang +":"+i+":"+SAXGetChars
            end if
            SAXSeekElement ( 0 )
            SAXSeekElement ( 0 )
            SAXSeekElement ( 0 )
         next y
'      end if
      SAXSeekElement ( 0 )
   next i
   if bCloseLocal = TRUE then SAXRelease
end function

