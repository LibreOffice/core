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
'* owner : oliver.craemer@oracle.com
'*
'* short description : Check content.xml and styles.xml for correct print scaling settings
'*
'*************************************************************************************************
'*
' #1 subPrintScaling
' #1 tPrintScaling
'*
'\*************************************************************************************************
sub subPrintScaling
  printLog Chr(13) + "---------  Print scaling attributes  ----------"

     call tPrintScaling

end sub

'************************************************************
testcase tPrintScaling
    dim iIndex as INTEGER
    dim iInnerIndex as INTEGER
    
    dim sExtension
    sExtension = ".ods"

    dim sOutputFile as STRING
    sOutputFile = ( gOfficePath & ConvertPath( "user\work\printScaling" ) & sExtension )
    
    dim sUnpackedStorageDir as STRING
    sUnpackedStorageDir = ( gOfficePath & ConvertPath( "user\work\printScaling" ))
    if (dir ( sUnpackedStorageDir ) <> "" ) then
        rmDir ( sUnpackedStorageDir )
        if (dir ( sUnpackedStorageDir ) <> "" ) then
            warnlog "The diretory has not been deleted. Exiting test!"
            goto endsub
        endif
    endif
    
    dim sContentXML as STRING
    sContentXML = ( sUnpackedStorageDir & ConvertPath( "\content.xml" ))
    
    dim sStylesXML as STRING
    sStylesXML = ( sUnpackedStorageDir & ConvertPath( "\styles.xml" ))
    
    dim asTableStyleAssocations ( 5 , 3 ) as STRING

    dim sNameSheet1 as STRING
    sNameSheet1 = "Ten"
    dim sNameSheet2 as STRING
    sNameSheet2 = "Moves"
    dim sNameSheet3 as STRING
    sNameSheet3 = "Ahead"
    
    dim sNameStyle1 as STRING
    sNameStyle1 = "1stStyle"
    dim sNameStyle2 as STRING
    sNameStyle2 = "2ndStyle"
    dim sNameStyle3 as STRING
    sNameStyle3 = "3rdStyle"

    dim iTablesInDocument as INTEGER
    dim iStylesInAutomaticStyles as INTEGER
    dim iMasterPageStylesInStylesXML as INTEGER
    dim sCurrentTableName as STRING
    dim sExpectedTableName as STRING
    dim sCurrentTableStyle as STRING
    dim sCurrentAutomaticStyle as STRING
    dim sCurrentMasterPageStyle as STRING

    dim sExpectedScaleTo      as STRING
    dim sExpectedScaleToX     as STRING
    dim sExpectedScaleToY     as STRING
    dim sExpectedScaleToPages as STRING
        
    printlog "Create a new spreadsheet document"
    Call hNewDocument
    sleep(2)
    
    for iIndex = 1 to 3
        printlog "Type in the first cell the number 1 and press RETURN"
        Kontext "DocumentCalc"
        DocumentCalc.TypeKeys "1<RETURN>"
        sleep(1)
        printlog "Format / Sheet / Rename"
        FormatSheetRename
        Kontext "TabelleUmbenennen"
        select case iIndex
               case 1 : Tabellenname.SetText sNameSheet1
                        printlog " In first round name it 'Ten'"                                              
               case 2 : Tabellenname.SetText sNameSheet2
                        printlog " In first round name it 'Moves'"
               case 3 : Tabellenname.SetText sNameSheet3
                        printlog " In first round name it 'Ahead'"
        end select
        printlog "Press OK on dialog" 
        TabelleUmbenennen.OK
        sleep(1)        
        printlog "Edit / Sheet / Select"
        EditSheetSelect
        Kontext "SelectSheets"
        printlog "In round 1 and 2 select the next sheet."
        printlog "In round three select the first sheet."
        if iIndex = 3 then            
            SheetSelectionBox.Select sNameSheet1
        else             
            SheetSelectionBox.Select ( iIndex + 1 )
        endif
        printlog "Press OK on dialog."
        SelectSheets.OK
        sleep(1)
    next iIndex
    printlog "Format / Styles / Catalog" 
    FormatStylesCatalog    
    Kontext "Vorlagenkatalog"
    Vorlagen.select ( 2 )
    sleep(1)    
    for iIndex = 1 to 3
        Kontext "Vorlagenkatalog"        
        printlog "Click new"
        Neu.Click
        Kontext        
        printlog "Page format dilog shoud appear)"        
        printlog "Select Page 'Organizer'"
        Kontext
        Active.SetPage TabVerwalten        
        printlog "Close MsgBox when appeared"
        if Active.Exists(2) then
            if Active.GetRT = 304 then
                Printlog Active.GetText
                Active.Yes
            end if
        end if
        Kontext "TabVerwalten"        
        printlog "Assign name for each style"
        select case iIndex                               
               case 1 : VorlagenName.setText sNameStyle1
                        printlog "First round name: '1stStyle'"
               case 2 : VorlagenName.setText sNameStyle2
                        printlog "Second round name:'2ndStyle'"
               case 3 : VorlagenName.setText sNameStyle3
                        printlog "Third round name: '3rdStyle'"
        end select                
        printlog "Select Page 'Sheet'"
        Kontext
        Active.SetPage TabTabelleCalc
        Kontext "TabTabelleCalc"                       
        scalingMode.Select ( iIndex )
        select case iIndex                              
               case 1 : SkalierungProzent.setText "55%"
                        printlog "1. round: Scale size to 55%"               
               case 2 : ScalePageWidth.setText "3"
                        ScalePageHeight.setText "4"                        
                        printlog "2. round: Scale output to 3 pages wide and 4 pages high"               
               case 3 : AnzahlMaxSeiten.setText "6"                        
                        printlog "3 round: Scale output to a total of 6 pages"
        end select        
        printlog "Leave page format dilog using OK"
        TabTabelleCalc.OK        
    next iIndex    
    printlog "Close style catalog using OK"
    Kontext "Vorlagenkatalog"
    Vorlagenkatalog.OK
    sleep(2)        
    for iIndex = 1 to 3            
        printlog "Edit / Sheet / Select"
        EditSheetSelect
        Kontext "SelectSheets"
        printlog "Select 1, 2. or 3 sheet"
        SheetSelectionBox.Select ( iIndex  )
        printlog "OK"
        SelectSheets.OK
        sleep(1)        
        Kontext "Gestalter"
        if NOT Gestalter.exists(2) then
            printlog "Format / Stylist"
            FormatStylist
        endif        
        Kontext "Gestalter"
        printlog "Click on 'Page styles'"
        Seitenvorlagen.Click
        Kontext "Vorlagenliste"
        printlog "Type [HOME]-key"
        Vorlagenliste.TypeKeys "<HOME>"
        sleep (2)
        printlog "Select 1, 2, or 3 (depends on the round)"
        Vorlagenliste.Select ( iIndex )
        printlog "Type [RETURN]"
        Vorlagenliste.TypeKeys "<RETURN>"
    next iIndex    
    Kontext "Gestalter"
    if Gestalter.exists(2) then
        printlog "Format / Stylist"
        FormatStylist
    endif            
    printlog "Save in current default ODF format"
    if hFileSaveAsWithFilterKill ( sOutputFile , "calc8" ) = FALSE then
       warnlog "Saving " & sOutputFile & " failed! -> Exiting test!"
       call hCloseDocument
    else       
       printlog "Close document"
       call hCloseDocument
       sleep(3)       
       printlog "Unpack storage (=compressed XML files) to obtain the XML files"       
       UnpackStorage ( sOutputFile , sUnpackedStorageDir )
       printlog "Read 'content.xml' DOM."       
       printlog "Read DOM of content.xml"
       SAXReadFile ( sContentXML )
       sleep(2)       
       printlog "Set the DOM pointer on element 'office:spreadsheet'"
       printlog "|-/"
       SAXSeekElement ( "/" )
       printlog "  |-office:document-content"
       SAXSeekElement ( "office:document-content" )
       printlog "    |-office:body"
       SAXSeekElement ( "office:body" )
       printlog "      |-office:spreadsheet"
       SAXSeekElement ( "office:spreadsheet" )
       iTablesInDocument = SAXGetChildCount       
       printlog "Check correct number of sheets"
       if iTablesInDocument <> 3 then
           warnlog "Unexpected number of tables in the document -> Check this out!"
       endif       
       printlog " nodes table:table"
       for iIndex = 1 to iTablesInDocument
            printlog "        |-table:table"
            SAXSeekElement ( "table:table" , iIndex )                        
            printlog "Get 'table:name' and 'table:style-name' for all tables"
            sCurrentTableName  = SAXGetAttributeValue ( "table:name" )
            sCurrentTableStyle = SAXGetAttributeValue ( "table:style-name" )
            select case iIndex
                   case 1 : sExpectedTableName = sNameSheet1
                   case 2 : sExpectedTableName = sNameSheet2
                   case 3 : sExpectedTableName = sNameSheet3
            end select            
            printlog "Check correct table names"
            if sCurrentTableName <> sExpectedTableName then
                warnlog "OOPS, this table has got an unexpected name -> Check this out!"
            endif            
            printlog "Memorize associations of table and style names"
            asTableStyleAssocations ( iIndex , 0 ) = sCurrentTableName
            asTableStyleAssocations ( iIndex , 1 ) = sCurrentTableStyle
            SAXSeekElement ( 0 )
       next iIndex
       SAXSeekElement ( 0 )
       SAXSeekElement ( 0 )       
       printlog "Go back in DOM-tree to 'office:automatic-styles'"
       printlog "        |-office:automatic-styles"
       SAXSeekElement( "office:automatic-styles" )
       iStylesInAutomaticStyles = SAXGetChildCount        
       if iStylesInAutomaticStyles < 5 then
           warnlog "Unexpected number of styles in 'content.xml' -> Check this out!"
       endif
       for iIndex = 1 to iStylesInAutomaticStyles       
           printlog "Seek nodes style:style"
           printlog "          |-office:automatic-styles"
           SAXSeekElement( "style:style" , iIndex )
           if SAXGetAttributeValue ( "style:family" ) = "table" then
               sCurrentAutomaticStyle = SAXGetAttributeValue ( "style:name" )
               printlog "            + Found table style " & sCurrentAutomaticStyle & " at postion " & iIndex               
               printlog "Memorize associations of table and master pages names using previously determined style names"
               for iInnerIndex = 1 to iTablesInDocument
                   if sCurrentAutomaticStyle = asTableStyleAssocations ( iInnerIndex , 1 ) then
                       asTableStyleAssocations ( iInnerIndex , 2 ) = SAXGetAttributeValue ( "style:master-page-name" )
                   endif
               next iInnerIndex
           endif
           SAXSeekElement ( 0 )
       next iIndex       
       printlog "Release DOM of 'content.xml'"
       SAXRelease
       printlog "| <<< DOM released <<<"
       printlog ""
       
       printlog "Read 'styles.xml' DOM."       
       printlog "Read DOM of styles.xml"
       SAXReadFile ( sStylesXML )
       sleep(2)
       printlog "Set the DOM pointer on element 'office:master-styles'"
       printlog "|-/"
       SAXSeekElement ( "/" )
       printlog "  |-office:document-styles"
       SAXSeekElement ( "office:document-styles" )
       printlog "    |-office:master-styles"
       SAXSeekElement ( "office:master-styles" )
       iMasterPageStylesInStylesXML = SAXGetChildCount       
       printlog "Seek nodes style:master-page"
       for iIndex = 1 to iMasterPageStylesInStylesXML
           printlog "      |-style:master-page"
           SAXSeekElement( iIndex )
           sCurrentMasterPageStyle = SAXGetAttributeValue ( "style:name" )
           printlog "        + Found table style " & sCurrentMasterPageStyle & " at postion " & iIndex
           for iInnerIndex = 1 to iTablesInDocument               
               printlog "Memorize associations of table names and master page styles using master page names found in 'context.xml'"
               if sCurrentMasterPageStyle = asTableStyleAssocations ( iInnerIndex , 2 ) then
                   if SAXGetElementName <> "style:master-page" then
                       warnlog "Unexpected style type found -> Check this out!"
                       qaErrorLog SAXGetElementName & " instead of 'style:master-page'"
                   endif
                   asTableStyleAssocations ( iInnerIndex , 3 ) = SAXGetAttributeValue ( "style:page-layout-name" )                   
                   printlog "Check correct UI strings for master page styles"
                   select case SAXGetAttributeValue ( "style:display-name" )
                          case sNameStyle1 : sExpectedTableName = sNameSheet1
                          case sNameStyle2 : sExpectedTableName = sNameSheet2
                          case sNameStyle3 : sExpectedTableName = sNameSheet3
                   end select                    
                   printlog "Check if UI strings for master page styles are correctly assigned to the three sheets"
                   if sExpectedTableName <> asTableStyleAssocations ( iInnerIndex , 0 ) then
                       warnlog "Association of UI Style name and table doesn't meet expectation -> Check this out!"
                       qaErrorLog "Expected '" & sExpectedTableName & "' but found '" & asTableStyleAssocations ( iInnerIndex , 0 ) & "'"
                   endif
               endif
           next iInnerIndex
           SAXSeekElement ( 0 )
       next iIndex
       SAXSeekElement ( 0 )       
       printlog "Go back in DOM-tree to 'office:automatic-styles'"
       printlog "      |-office:automatic-styles"
       SAXSeekElement ( "office:automatic-styles" )
       iStylesInAutomaticStyles = SAXGetChildCount       
       printlog "Seek nodes style:page-layout"
       for iIndex = 1 to iStylesInAutomaticStyles
           printlog "      |-style:page-layout"
           SAXSeekElement (  iIndex )
           sCurrentAutomaticStyle = SAXGetAttributeValue ( "style:name" )                      
           printlog "Check for each sheet if expected page scaling is correctly assigned"
           for iInnerIndex = 1 to iTablesInDocument
               if sCurrentAutomaticStyle = asTableStyleAssocations ( iInnerIndex , 3 ) then
                   if SAXGetElementName <> "style:page-layout" then
                       warnlog "Unexpected style type found -> Check this out!"
                       qaErrorLog SAXGetElementName & " instead of 'style:page-layout'"
                   endif
                   printlog "        |-style:page-layout"
                   SAXSeekElement ( "style:page-layout-properties" )
                   select case asTableStyleAssocations ( iInnerIndex , 0 )
                          case sNameSheet1
                               sExpectedScaleTo      = "55%"
                               sExpectedScaleToX     = ""
                               sExpectedScaleToY     = ""
                               sExpectedScaleToPages = ""
                          case sNameSheet2
                               sExpectedScaleTo      = ""
                               sExpectedScaleToX     = "3"
                               sExpectedScaleToY     = "4"
                               sExpectedScaleToPages = ""
                          case sNameSheet3
                               sExpectedScaleTo      = ""
                               sExpectedScaleToX     = ""
                               sExpectedScaleToY     = ""
                               sExpectedScaleToPages = "6"
                   end select
                   if sExpectedScaleTo <> SAXGetAttributeValue ( "style:scale-to") then
                       warnlog " Attribute 'style:scale-to' has unexpected value: " & SAXGetAttributeValue ( "style:scale-to")
                       qaErrorLog "Expected: " & sExpectedScaleTo
                   endif
                   if sExpectedScaleToX <> SAXGetAttributeValue ( "style:scale-to-X") then
                       warnlog "Attribute 'style:scale-to-X' has unexpected value: " & SAXGetAttributeValue ( "style:scale-to-X")
                       qaErrorLog "Expected: " & sExpectedScaleToX
                   endif
                   if sExpectedScaleToY <> SAXGetAttributeValue ( "style:scale-to-Y") then
                       warnlog "Attribute 'style:scale-to-Y' has unexpected value: " & SAXGetAttributeValue ( "style:scale-to-Y")
                       qaErrorLog "Expected: " & sExpectedScaleToY
                   endif
                   if sExpectedScaleToPages <> SAXGetAttributeValue ( "style:scale-to-pages") then
                       warnlog "Attribute 'style:scale-to-pages' has unexpected value: " & SAXGetAttributeValue ( "style:scale-to-pages")
                       qaErrorLog "Expected: " & sExpectedScaleToPages
                   endif
                   SAXSeekElement ( 0 )
               endif
           next iInnerIndex
           SAXSeekElement ( 0 )
       next iIndex
    endif    
    printlog "Release DOM of 'styles.xml'"
    SAXRelease
endcase

