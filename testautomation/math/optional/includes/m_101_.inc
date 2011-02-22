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
'\******************************************************************

 dim cActFilter as string ' used filter for save
 Const cActFilterExt = ".sxm" ' used filter for save (extension)

sub m_101_
   cActFilter = gMathFilter
   Printlog Chr(13) + "--  File Menu  --"

' tFileNew
'   Call Autopilot               'in Inc\desktop\autopilo.inc
'   Call tFileClose
  Call tFileSave
  Call tFileSaveAs
  Call tFilePassword ' check which filtertypes enable saving with password
  Call tFileOpen_1   ' just load all previous saved files in this test
  Call tFileOpen_2   ' load testfiles for filters
    gApplication = "WRITER"
    Call tToolsCatalog  ' code is in m_105_.inc, but i want to print he result, so i needed to moove teh call to here
    gApplication = "MATH"
  Call tFilePrint    ' writer dokument with math ole's
'   Call tFileSaveAll
'   Call tFileNewLoad
'   Call tFileVersions
' tDocumentAsEmail
'   Call tFileProperties
'   Call tFilePrintersetting
'   Call tFileEnd

end sub

testcase tFileSave
   dim sFileName as string ' test document & new created doc
   dim sFormula as string  ' container for formula to create document with
   dim Exlist(30) as string
   dim sTemp as string
   dim i,x,y as integer

  '/// if not exists : gOfficePath + '\\user\\math\\level1\\', create it ///'
  if app.dir (ConvertPath ( gOfficePath + "user/work/math") ) = "" then
     app.mkdir ConvertPath ( gOfficePath + "user/work/math")
  endif
  if app.dir (ConvertPath ( gOfficePath + "user/work/math/level1")) = "" then
     app.mkdir ConvertPath ( gOfficePath + "user/work/math/level1")
  endif

  sFileName = ConvertPath ( gOfficePath + "user/work/math/level1")
  Printlog "'/// deleting all files in output directory: \\user\\work\\math\\level1\\ ///'"
   if gSamePC = TRUE then ' delete export directory
      GetFileList ( sFileName, "*.*", Exlist() )
      if KillFileList ( Exlist() ) <> TRUE then
         Warnlog "Couldn't delete all Files in Output-Export-Directory, the followings are still there:"
         for i=1 to ListCount ( Exlist() )
            printlog "  <> " + Exlist(i)
         next i
      end if
   end if

' should be default format?!

'/// take formulatext from loaded file -> language independant! ///'
   sFormula = TBOloadFormulaFromFile()
   if sFormula = "gehtnicht" then
       qaErrorlog "can not make document writeable - exiting test"
       goto endsub
   endif
   hNewDocument
'///  write formula down  ///'
   SchreibenInMathdok (sFormula)
   sleep 3
   hFileSaveAsKill (sFileName + "filesave.sxm")
' Get used filter name
   FileSaveAs
   sleep 2
   Kontext "SpeichernDlg"
   sTemp = Dateityp.GetSelText
   SpeichernDlg.Cancel

'/// file save with default filter as: "\\user\\work\\math\\level1\\filesave...." ///'

   printlog " saved with filter: "+ sTemp
   Call hCloseDocument
endcase

testcase tFileSaveAs
   dim sFileName as string ' test document & new created doc
   dim sFormula as string  ' container for formula to create document with
   dim sTemp as string
   dim sFilter (50) as string
   dim i,x,y as integer
   dim bAlienWarning as boolean

   sFilter (0) = 0 ' initalisize ;-)...

'/// take formulatext from loaded file -> language independant! ///'
   sFormula = TBOloadFormulaFromFile
   if sFormula = "gehtnicht" then
       qaErrorlog "can not make document writeable - exiting test"
       goto endsub
   endif

   hNewDocument
'///  write formula down  ///'
   SchreibenInMathdok sFormula
   sleep 3

'/// start loop 3 times for: ///'
'///+ StarOffice 6.0 Formula ///'
'///+ StarMath 5.0 ///'
'///+ MathML 1.01 ///'
'/// to be language independant, select first, second last and last filter from list -> but try to check the names :-) ///'

' to use the helper fileSaveAs functions i get all available filters...
      ListAppend(sFilter(), "StarOffice XML (Math)")
      ListAppend(sFilter(), "math8")

   bAlienWarning = isCheckedWarnAlienFormat
   if (NOT bAlienWarning) then
        qaErrorLog "Somebody disabled 'Alien Warnings' in the options, will enable it again now."
        isCheckedWarnAlienFormat(true)
   endif
   
   for i = 1 to (ListCount(sFilter()))
      sFileName = convertpath( gOfficePath + "user/work/math/level1/fsas_"+(i))
      sleep 1
      printlog "going to save: '"+sFileName+"'"
      sleep 1
      '----- try to WorkAround focus change ---------
      kontext "DocumentMath"
      DocumentMath.mouseDown 10,10
      DocumentMath.mouseUp 10,10
      '-----------------------------------------------
      hFileSaveAsWithFilterKill (sFileName, sFilter(i), TRUE )
      '/// file save with filter as: "output\\math\\fsas_[0,1,2]" ///'
      ' Alien Warning has not to appear on the first one: Default format .odf
      Kontext "AlienWarning"
      if AlienWarning.exists(5) then
          if (1=i) then
              warnlog "- AlienWarning on default file format! Not OK!"
          else
              printlog "- AlienWarning"
          endif
          AlienWarning.ok
      endif
      printlog " saved with filter ("+i+"/2): "+ sFilter(i)
   next i
'/// TODO: small check, if correct filter was used! ///'
   sleep 3
   fileclose
   sleep 3
   kontext "active"
   if active.exists then
      printlog "active about informationloss... :-) that's OK: '"+active.GetText+"'"
      active.yes
   endif
endcase

testcase tFileOpen_1
'/// a small loop, to load all files previous saved ///'
   dim sFileName as string ' test document & new created doc
   dim sFormula as string  ' container for formula to create document with
   dim sFileList (30) as string
   dim sTemp as string
   dim i,x,y as integer

'///load documents from  "output\\math\\" ///
   sFileName = ConvertPath ( gOfficePath + "user/work/math/level1/" )
   GetFileList ( sFileName, "*.*", sFileList() )

   x = ListCount ( sFileList() )
   for i = 1 to x
     printlog "("+i+"/"+x+"): "+sFileList(i)
      hFileOpen ( sFileList(i) )
      Sleep 2
      If hIsNamedDocLoaded (sFileList(i)) Then
         printlog "  used filter: " + hGetUsedFilter()
      else
         warnlog "document didn't get loaded"
      endif
      hCloseDocument
      sleep 2
   next i
endcase

testcase tFileOpen_2
'/// a small loop, to load a testfile for every non SO filter files ///'
'/// files are taken from: http://www.dessci.com/en/products/mathtype/win/samples/ ///'
   dim sFileName as string ' test document & new created doc
   dim sFormula as string  ' container for formula to create document with
   dim sFileList (30) as string
   dim sTemp as string
   dim i,x,y as integer

'///load documents from  "input\\math\\level1\\filter\\" ///
   sFileName = ConvertPath ( gTesttoolPath + "math/optional/input/level1/filter/" )
   GetFileList ( sFileName, "*.*", sFileList() )

   x = ListCount ( sFileList() )
   for i = 1 to x
     printlog "("+i+"/"+x+"): "+sFileList(i)
      hFileOpen ( sFileList(i) )
      Sleep 20
      If hIsNamedDocLoaded (sFileList(i)) Then
         printlog "  used filter: " + hGetUsedFilter()
      else
         warnlog "document didn't get loaded"
      endif
      hCloseDocument
   next i
endcase

testcase tFilePassword
   dim i,x as integer
   dim sTemp as string
   const cFilterCount = 3
   dim sFilterName as string
   dim sFilter as string
   dim sFilterExt() as string

   Printlog "'///- Check each Filter if password is possible ///"

   ' make sure saving in latest math filter works, checking global filtername and if necessary change it
   sFilter = hGetUIFiltername("math8")
   sFilterExt() = hGetFilternameExtension("math8")
   sFilterName = sFilter + " (." + sFilterExt(0) + ")"
   printlog sFilterName
   printlog gMathFilter
       gMathFilter = sFilterName

   hNewDocument
   SchreibenInMathdok "a over b"

   FileSaveAs
   Kontext "SpeichernDlg"

   '/// check amount of available filters (6) ///'
   x = Dateityp.GetItemCount
   if x <> cFilterCount then warnlog " The Filter count is wrong! Have to be ("+cFilterCount+"), but are: "+x
'   Check: (1): Open Office Formula (.oof)
'   Check: (2): MathML 1.01 (.mml)
'   Check: (6): StarOffice 6.0/7 Formula (.sxm)

'/// check if any other then gMathFilter is password able; and if gMathFilter is password able ///'
   for i = 1 to x
      Dateityp.Select i
      sleep (5)
      sTemp = Dateityp.GetSelText
      printlog "   Check: (" + i + "/" + x +"): " +sTemp
      if Passwort.isEnabled then
         if NOT((instr(sTemp,gMathFilter) = 1)OR(instr(sTemp," (.sxm)") > 1)) then 
             if (4=i) then
                '#i112895#
                warnlog "(" + i + "/" + x +"): '"+sTemp+"' Password Checkbox: Enabled"
             else
                warnlog "(" + i + "/" + x +"): '"+sTemp+"' Password Checkbox: Enabled"
            endif
         endif
      else
         if (instr(sTemp,gMathFilter) = 1)OR(instr(sTemp," (.sxm)") > 1) then 
             warnlog "(" + i + "/" + x +"): '"+sTemp+"' Password Checkbox: Disabled"
         endif
      endif
   next i

   SpeichernDlg.Cancel
   hCloseDocument
endcase

function TBOloadFormulaFromFile () as string
'/// load an existing document: "input\\math\\101_1.sxm" ///
'///+  take formula out of command window ///
'///+  save it into a variable ///
'///  close dokument ///
   Call hFileOpenLocally ( ConvertPath(gTesttoolPath + "math/optional/input/101_1.sxm") )
   kontext "dokumentwriter"
   try
   EditSelectAllMath
   EditCopy
   TBOloadFormulaFromFile = GetClipboardText
   catch
   TBOloadFormulaFromFile = "gehtnicht"
   endcatch
'   printlog "-----TBOloadFormulaFromFile: will return: '"+TBOloadFormulaFromFile+"'"
   hCloseDocument
end function

testcase tFilePrint
'/// I create a writer document and insert the object formula from the standard testfile: ///'
'///+ There will be a graphic, how it should look like and the object///'
'///+ AND there was the formula already inserted as object!///'
'///+ in different languages...///'
'/// Document is called: "input\\math\\101_2.sxw ///
'///+ Image is available at: "input\\math\\101_2.gif///'

    dim sFormula as string  ' container for formula to create document with
    dim sFileName as string
    dim bChecked as boolean
    dim sTemp as string
    dim sTemp1 as string
    dim bLanguage as boolean
    dim bString as boolean
    dim iLanguage as integer
    dim sFoundLanguage as string
    dim bError as boolean
    dim iTimeOut as integer

    printlog "- File Print"

    bError = FALSE
    sFormula = TBOloadFormulaFromFile()
   if sFormula = "gehtnicht" then
       qaErrorlog "can not make document writeable - exiting test"
       goto endsub
   endif

    Call hFileOpenLocally ( ConvertPath(gTesttoolPath + "math/optional/input/101_2.sxw") )

    '/// find the translated formula in the document and get the language it belongs to ///'
    '///+ compare it to the officelanguage ///'
    kontext "DocumentWriter"
        DocumentWriter.TypeKeys "<mod1 home>"
        bLanguage = hFindeImDokument("{"+ iSprache +"}")
    kontext "DocumentWriter"
        DocumentWriter.TypeKeys "<mod1 home>"
        hFindeImDokument ("\{"+ iSprache +"\}.*\{"+ iSprache +"\}", TRUE, TRUE)
        try
            editcopy
        catch
            printlog "editcopy failed."
            setClipboard("")
        endcatch
        sFoundLanguage = GetClipboardText
        If NOT bLanguage then
            qaErrorLog "This Language (" + iSprache + ") is not supported in this test right now! Add the following string to the document: " +ConvertPath(gTesttoolPath + "math/optional/input/101_2.sxw")
        else
            kontext "DocumentWriter"
            DocumentWriter.TypeKeys "<left>"
            if bError then
                DocumentWriter.TypeKeys "<right>"
            endif
            DocumentWriter.TypeKeys "<shift left><shift left>"
            editcopy
            stemp = GetClipboardText
            if (left(sTemp,1) = "{") then
                ' language number is monadic
                sTemp = right(sTemp,1)
            endif
            iLanguage = val(sTemp)
            if (iSprache <> iLanguage) then
                warnlog "hmm, smth is wrong :-(; found translated text for language: "+val(GetClipboardText) + ", but looked for: "+iSprache
            endif
        endif
    kontext "DocumentWriter"
        DocumentWriter.TypeKeys "<mod1 home>"
        bString = hFindeImDokument (sFormula)
        If NOT bString then
            if bLanguage then
                warnlog "The translation changed! If It is a ProductPatch, then this is a bug; else update the document by calling TBO.:" +ConvertPath(gTesttoolPath + "math/optional/input/101_2.sxw")
            endif
            printlog sFormula
        endif
        sleep 1
        try
            editcopy
        catch
            If (bString AND bLanguage) then
                warnlog "Edit-Copy failed."
            endif
        endcatch
    printlog "found.......................... '"+GetClipboardText+"'"
    printlog "looked for..................... '"+sFormula+"'"
    printlog "found language............. '"+sFoundLanguage+"'"
    if (GetClipboardText <> sFormula) then
        If (bString AND bLanguage) then
            warnlog "difference to reference!"
        endif
    else
        printlog "FOUND entry for the language!: '"+ iLanguage +"'"
    endif

    kontext "DocumentWriter"
    DocumentWriter.TypeKeys "<mod1 home><down><down>"
    DocumentWriter.TypeKeys gPCname + " " + iSprache + " " + gLanguage + " " + gPlatgroup + " " + gPlatform + "<return>"
    '/// Insert the loaded and automatical 'translated' formulatext in writer as text ///'
    hFindeImDokument ("TBO_INSERTED")
    kontext "DocumentWriter"
    DocumentWriter.TypeKeys sFormula
    DocumentWriter.TypeKeys "<mod1 end>"
    
    sleep (2)
    '/// call: Insert->Object->Formula ///'
    InsertObjectFormulaWriter
    '///  write formula down  ///'
    SchreibenInMathdok (sFormula)
    sleep 3
    '/// exit formula object ///'
    kontext "DokumentWriter"       'Commands'CommandsMath'DocumentMath don't work :-[
    DokumentWriter.MouseDown 99,99
    DokumentWriter.MouseUp 99,99
    DokumentWriter.MouseDown 99,99
    DokumentWriter.MouseUp 99,99

    sFileName = convertpath( gOfficePath + "user/work/math/level1/ftc_a.odt")
    if (FileExists(sFileName)) then
        '/// goto end of page; insert page break ///'
        kontext "DokumentWriter"
        DokumentWriter.typeKeys "<mod1 end>"
        sleep 1
        DokumentWriter.typeKeys "<mod1 end>"
        sleep 1
        DokumentWriter.typeKeys "<mod1 end>"
        sleep 1
        DokumentWriter.typeKeys "<mod1 return>"
        sleep 1
        '/// insert frame ///'
        try
            InsertFrame
        catch
            printlog "Catched because of failed mouse action, try to leafe math in oposite corner"
            kontext "DokumentWriter"
            DokumentWriter.MouseDown 1,1 ' TBO: i don't like to do it this way,
            DokumentWriter.MouseUp 1,1   '      but using .TypeKeys doesn't work
            DokumentWriter.MouseDown 1,1 '      that way i expected in math via ole :-[
            DokumentWriter.MouseUp 1,1
            kontext "DokumentWriter"
            DokumentWriter.typeKeys "<mod1 end>"
            sleep 1
            DokumentWriter.typeKeys "<mod1 end>"
            sleep 1
            DokumentWriter.typeKeys "<mod1 end>"
            sleep 1
            DokumentWriter.typeKeys "<mod1 return>"
            sleep 1
            '/// insert frame ///'
            InsertFrame
        endcatch
        sleep 1
        Kontext
        active.setPage TabType
        kontext "TabType"
            RelativeWidth.check
            RelativeHeight.check
            Width.setText "100"
            Height.setText "98"
            RelativeWidth.unCheck
            RelativeHeight.unCheck
            AutoHoehe.Check
        TabType.OK
        kontext "DokumentWriter"
        sleep 1
        DokumentWriter.typeKeys "<f2>"
        sleep 1
        
        '/// load file from testcase 'tToolsCatalog' to get a hardcopy ///'
        InsertFileWriter
            Kontext "GeneralFileDialog"
            Dateiname.setText sFileName
            Oeffnen.click
        sleep 60
        
        '/// do some nice formatting ///'
        FormatColumns
            Kontext "FormatColumn"
            Columns.setText "2"
            FormatColumn.TypeKeys "<tab>"
            iTimeOut = 1
            while ((NOT AutoWidth.isEnabled) AND (iTimeOut < 30))
                sleep 1
                printlog "Waiting for control to be enabled: (" + iTimeOut + "/30)"
                iTimeOut = iTimeOut +1
                Kontext "FormatColumn"
                FormatColumn.TypeKeys "<tab>"
            wend
            Kontext "FormatColumn"
            bChecked = AutoWidth.isChecked
            AutoWidth.UnCheck
            sTemp = Width1.getText
            Width1.typeKeys("<PageDown>")
            Width1.more
            sTemp1 = Width1.getText
            Width1.setText(sTemp)
            if (bChecked) then
                AutoWidth.UnCheck
            endif
            FormatColumn.OK
        
        FormatParagraph
        Kontext
        active.setPage TabTabulator
            kontext "TabTabulator"
            Position.setText sTemp
            Neu.click
            Position.setText sTemp1
            Neu.click
            try
                TabTabulator.OK
            catch
                warnlog "Timeout on closing "
                iTimeOut = 1
                while ((iTimeOut > 0) AND (iTimeOut < 30))
                    try
                        Kontext "DocumentWriter"
                        DocumentWriter.typeKeys "<right>"
                        iTimeOut = iTimeOut * (-1)
                    catch
                        sleep (10)
                        inc(iTimeOut)
                    endcatch
                wend
                printlog "Duration: " + (iTimeOut * 10)
            endcatch
    else
        qaErrorLog "File doesn't exists: '" + sFileName + "'"
    endif
    
    '/// print file ///'
    try
        Fileprint
        kontext
        if active.exists(2) then
            active.ok
            qaerrorlog "There is no printer available - please install one on your system!"
            Kontext "Printing"
            if ( Printing.exists( 2 ) ) then
                Printing.cancel
            else
                warnlog( "Printing dialog did not open" )
            endif
        else
            Kontext "Printing"
            if ( Printing.exists( 2 ) ) then
                Printing.ok
            else
                warnlog( "Printing dialog did not open" )
            endif
        endif

        kontext
        if active.exists(2) then
            active.ok
            qaerrorlog "There is no printer available - please install one on your system!"
            sleep 2
        endif
    catch
        warnlog "FilePrint is not accessible, because closing Formula failed :-( ?!"
    endcatch
    hFileSaveAsWithFilterKill (convertpath( gOfficePath + "user/work/fileprint" + iSprache + ".odt"), "writer8")
    Call hCloseDocument
endcase


function isCheckedWarnAlienFormat (optional bChange as boolean)
    dim uno
    dim ap
    dim apara(1) As new com.sun.star.beans.PropertyValue
    dim xViewRoot
    dim temp()
    dim bLocal as boolean
    dim sFileFunction as string
    dim i as integer

    if isMissing(bChange) then
    '   should work, else give soffice the commandline
        try
            uno=getUnoApp
        catch
            qaErrorLog (sFileFunction+"UNO-Port wasn't set on startup of office; will set it now again.")
            uno=hGetUnoService()
        endcatch
        if (isNull(ap)) then
            warnlog(sFileFunction+"Couldn't create Uno access")
            ' think about switching to do it manual?
            exit function
        endif
        ap=uno.createInstance("com.sun.star.configuration.ConfigurationProvider")
        if (isNull(ap)) then
            warnlog(sFileFunction+"Couldn't create Configuration access")
            exit function
        endif
        apara(0).Name="nodepath"
        apara(1).Name="lazywrite"
        apara(1).Value=False
    '() Tools->Options->Load/Save->General
        '/org.openoffice.Office.Common/Save/Document WarnAlienFormat
        apara(0).Value="/org.openoffice.Office.Common/Save/Document"
        xViewRoot=ap.createInstanceWithArguments("com.sun.star.configuration.ConfigurationUpdateAccess",aPara())
        bLocal = xViewRoot.getByName("WarnAlienFormat")
        xViewRoot.dispose()
    else
        ToolsOptions
        call hToolsOptions ("LoadSave", "General")
        bLocal = WarnWhenSavingAlienFormat.isChecked
        if bChange then
            WarnWhenSavingAlienFormat.check
        else
            WarnWhenSavingAlienFormat.unCheck
        endif
        Kontext "ExtrasOptionenDlg"
        ExtrasOptionenDlg.OK
        sleep 4
    endif
    isCheckedWarnAlienFormat = bLocal
end function

function hSpeichernUnterMitFilterKillTBO (sFileName as String, sFilterName, optional bAutoFileExtension as boolean) as boolean
    Dim sDerName as string
    Dim sDerText as string
    Dim ApplicatStrg as string
    Dim bFileSaved as boolean
    Dim bAlienNotAllowed as boolean

    sDerName = ConvertPath (sFileName)
    if ( not hDeleteFile( sDerName ) ) then
        warnlog( "Aborting function because the file could not be deleted: " & sDerName )
        hSpeichernUnterMitFilterKillTBO() = FALSE
        exit function
    endif
 
    sleep (1)
    FileSaveAs    
    Kontext "SpeichernDlg"
    if SpeichernDlg.Exists(2) then
        DateiTyp.Select sFilterName
        sleep (3)
        DateiName.SetText sDerName
        sleep (3)
        'Saving the document
        Speichern.Click
        sleep (1)
        
        Kontext
        if Active.Exists(2) then
            if Active.GetRT = 304 then
                sDerText = Active.GetText
                warnlog sDerText                
                try
                    Active.OK
                    printlog "OK"
                catch
                    active.yes
                    printlog "YES"
                endcatch
                Kontext "SpeichernDlg"
                if SpeichernDlg.Exists(1) then                    
                    SpeichernDlg.Cancel
                end if
                bFileSaved = FALSE
            end if
        end if
        
    else
        warnlog "'Save'-dialog is not visible. Document has not been saved!"
    end if
end function


