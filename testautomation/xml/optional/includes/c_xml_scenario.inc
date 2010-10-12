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
'* short description : Check content.xml for correct scenario attributes
'*
'*************************************************************************************************
'*
' #1 subScenarioAttributes
' #1 tScenarioAttributes
' #1 subCreateScenario
' #1 subCheckScenarioInContentXML
'*
'\*************************************************************************************************
sub subScenarioAttributes
  printLog Chr(13) + "---------  scenario attributes  ----------"

     call tScenarioAttributes

end sub

'************************************************************
testcase tScenarioAttributes
    dim iIndex as INTEGER
    
    dim sExtension
    sExtension = ".ods"

    dim sNameSheet1 as STRING
    sNameSheet1 = "Ten"
    dim sNameSheet2 as STRING
    sNameSheet2 = "Moves"
    dim sNameSheet3 as STRING
    sNameSheet3 = "Ahead"

    
    dim sOutputFile as STRING
    sOutputFile = ( gOfficePath & ConvertPath( "user\work\scenarioAttributes" ) & sExtension )
    
    dim sUnpackedStorageDir as STRING
    sUnpackedStorageDir = ( gOfficePath & ConvertPath( "user\work\scenarioAttributes" ))
    if (dir ( sUnpackedStorageDir ) <> "" ) then
        rmDir ( sUnpackedStorageDir )
        if (dir ( sUnpackedStorageDir ) <> "" ) then
            warnlog "Diretory has not been deleted. Exiting test."
            goto endsub
        endif
    endif
    
    dim sContentXML as STRING
    sContentXML = ( gOfficePath & ConvertPath( "user\work\scenarioAttributes" ) & ConvertPath( "\content.xml" ))

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
    printlog "If not visible: View / Toolbars / Formula bar"
    Kontext "RechenleisteCalc"
    if not RechenleisteCalc.isvisible then
        ViewToolbarsFormulaBar
    end if
    sleep(1)    
    Kontext "RechenleisteCalc"
    printlog "In the Formular bar edit area type:"
    Bereich.TypeKeys "<MOD1 A>"
    printlog "[CTRL+A]"    
    Bereich.TypeKeys "C3"
    printlog "C3"
    sleep(1)
    printlog "[RETURN]"
    Bereich.TypeKeys "<RETURN>"
    sleep(1)
    Kontext "DocumentCalc"
    printlog "Type in the spreadsheet document:"
    printlog "1[RETURN]2[RETURN]3[RETURN]"
    DocumentCalc.typekeys "1<RETURN>2<RETURN>3<RETURN>"    
    Kontext "RechenleisteCalc"
    printlog "In the Formula bar type in the edit area:"
    printlog "[CTRL+A]"
    Bereich.TypeKeys "<MOD1 A>"
    printlog "C3:C5"
    Bereich.TypeKeys "C3:C5"
    sleep(1)
    printlog "[RETURN]"
    Bereich.TypeKeys "<RETURN>"
    sleep(1)
    printlog "Tools / scenarios..."
    printlog "Scenario name: Scenario_XML_Test"
    printlog "Scenario comment: Created by Peter Junge"
    printlog "Color index: 1"
    printlog "Display border unchecked"
    printlog "Copy back unchecked"
    printlog "Copy entire sheet unchecked"
    printlog "Prevent changes unchecked"
    subCreateScenario ( "Scenario_XML_Test" , "Created by Peter Junge" , 1 , 0 , 0 , 0 , 0 )
    sleep(1)    
    printlog "Create a another scenario on this selection with the following parameters:"
    printlog "Scenario name: AnotherScenario"
    printlog "Scenario comment: Random Comment"
    printlog "Color index: 13"
    printlog "Display border checked"
    printlog "Copy back checked"
    printlog "Copy entire sheet checked"
    printlog "Prevent changes checked"        
    subCreateScenario ( "AnotherScenario" , "Random Comment" , 13 , 1 , 1 , 1 , 1 )
    sleep(1)
    printlog "Edit / Sheet / Select"
    EditSheetSelect
    Kontext "SelectSheets"
    printlog "Select 'Moves'"
    SheetSelectionBox.Select "Moves"
    printlog "OK"
    SelectSheets.OK
    sleep(1)
    printlog "Type in Formula bar:"    
    Kontext "RechenleisteCalc"
    printlog "[CTRL+A]"
    Bereich.TypeKeys "<MOD1 A>"
    printlog "d5"
    Bereich.TypeKeys "d5"
    sleep(1)
    printlog "[RETURN]"
    Bereich.TypeKeys "<RETURN>"
    sleep(1)
    printlog "Type in spreadsheet document"
    Kontext "DocumentCalc"
    printlog "1[RETURN]2[RETURN]3[RETURN]"
    DocumentCalc.typekeys "1<RETURN>2<RETURN>3<RETURN>"    
    Kontext "RechenleisteCalc"
    printlog "Type in Formula bar:"
    printlog "[CTRL+A]"
    Bereich.TypeKeys "<MOD1 A>"
    printlog "d5:d7"
    Bereich.TypeKeys "d5:d7"
    sleep(1)
    printlog "[RETURN]"
    Bereich.TypeKeys "<RETURN>"
    sleep(1)
    printlog "Creat another scenario:"
    subCreateScenario ( "標準" , "기본값" , 9 , 1 , 1 , 0 , 0 )
    printlog "Scenario name: 標準"
    printlog "Scenario comment: 기본값"
    printlog "Color index: 9"
    printlog "Display border checked"
    printlog "Copy back checked"
    printlog "Copy entire sheet unchecked"
    printlog "Prevent changes unchecked"            
    sleep(1)
    '///<li>Create a another scenario on this selection</li>///
    subCreateScenario ( "1标准9" , "2預設0" , 16 , 0 , 0 , 1 , 1 )
    printlog "Scenario name: 1标准9"
    printlog "Scenario comment: 2預設0"
    printlog "Color index: 16"
    printlog "Display border unchecked"
    printlog "Copy back unchecked"
    printlog "Copy entire sheet checked"
    printlog "Prevent changes checked"            
    sleep(1)        
    printlog "Save in current default format"
    if hFileSaveAsWithFilterKill ( sOutputFile , "calc8" ) = FALSE then
       warnlog "Saving " & sOutputFile & " failed! -> Exiting test!"
       call hCloseDocument
    else
       printlog "Close the document."
       call hCloseDocument
       sleep(3)       
       printlog "Unpack storage of test document."
       UnpackStorage( sOutputFile , sUnpackedStorageDir )
       printlog "Read the test documents DOM."       
       SAXReadFile( sContentXML )
       sleep(2)       
       printlog "Set the DOM pointer on element 'office:spreadsheet'"
       printlog "|-/"
       SAXSeekElement( "/" )
       printlog "  |-office:document-content"
       SAXSeekElement( "office:document-content" )
       printlog "    |-office:body"
       SAXSeekElement( "office:body" )
       printlog "      |-office:spreadsheet"
       SAXSeekElement( "office:spreadsheet" )       
       printlog "For all tables check if they are scenarios"       
       printlog "For those tables which are scenarios check correct attributes"
       for iIndex = 1 to 7
           printlog "        |-table:table"
           SAXSeekElement( "table:table" , iIndex )
           select case iIndex
                  case 1 : subCheckScenarioInContentXML ( "Ten" , FALSE )
                  case 2 : subCheckScenarioInContentXML ( _
                           "Scenario_XML_Test" , TRUE , TRUE , "#000000" , TRUE , "false" , FALSE , "false" , _
                           "Scenario_XML_Test.C3:Scenario_XML_Test.C5" , "Created by Peter Junge" )
                  case 3 : subCheckScenarioInContentXML ( _
                           "AnotherScenario" , TRUE , FALSE , "#ff0000" , FALSE , "false" , TRUE , "true" , _
                           "AnotherScenario.C3:AnotherScenario.C5" , "Random Comment" )
                  case 4 : subCheckScenarioInContentXML ( "Moves" , FALSE )
                  case 5 : subCheckScenarioInContentXML ( _
                           "標準" , TRUE , FALSE , "#c0c0c0" , FALSE , "false" , FALSE , "false" , _
                           "標準.D5:標準.D7" , "기본값" )
                  case 6 : subCheckScenarioInContentXML ( _
                           "1标准9" , TRUE , TRUE , "#ffffff" , TRUE , "false" , TRUE , "true" , _
                           "1标准9.D5:1标准9.D7" , "2預設0" )
                  case 7 : subCheckScenarioInContentXML ( "Ahead" , FALSE )
           end select           
           SAXSeekElement( 0 )
       next iIndex       
       printlog "Check if too much sheets exist"
       try 
           SAXSeekElement( "table:table" , 8 )
           warnlog "OOPS, it's expected to have only seven sheet but we got eight or more -> Check this out!"
           SAXSeekElement( 0 )
       catch           
       endcatch
    endif
    SAXRelease
endcase
'
'---------------------------------------------------------------------------
'
sub subCreateScenario ( sScenarioName as STRING, _
                        sComment as STRING, _
                        iColorIndex as INTEGER, _
                        bDisplayBorder as BOOLEAN, _
                        bCopyBack as BOOLEAN, _
                        bCopyEntireSheet as BOOLEAN, _
                        bPreventChanges as BOOLEAN )
        
        printlog "tools / scenarios..."
        ToolsScenarios
        Kontext "SzenarioAnlegen"        
        printlog "Set options as requested through arguments"
        SzenarioName.SetText ( sScenarioName )
        Kommentar.SetText ( sComment )
        Rahmenfarbe.Select ( iColorIndex )
        if bDisplayBorder then
            RahmenAnzeigen.Check
        else
            RahmenAnzeigen.UnCheck
        endif
        if bCopyBack then
            Zurueckkopieren.Check
        else
            Zurueckkopieren.UnCheck
        endif
        if bCopyEntireSheet then
            GanzeTabelle.Check
        else
            GanzeTabelle.UnCheck
        endif
        if bPreventChanges then
            PreventChanges.Check
        else
            PreventChanges.UnCheck
        endif
        printlog "OK"        
        SzenarioAnlegen.OK
        sleep(1)	
end sub
'
'---------------------------------------------------------------------------
'
sub subCheckScenarioInContentXML ( sTableName as STRING , _
                                   bTableShouldBeScenario as BOOLEAN , _
                                   optional bCheckDisplayBorder as BOOLEAN , _
                                   optional sBorderColor as STRING , _
                                   optional bCheckCopyBack as BOOLEAN , _
                                   optional sCopyStyles as STRING , _
                                   optional bCheckProtected as BOOLEAN , _
                                   optional sIsActive as STRING , _
                                   optional sScenarioRanges as STRING , _
                                   optional sComment as STRING)
                                   
'///<u><b>Check attibutes for scenarios in 'content.xml'</b></u>///
dim bTableIsScenario as BOOLEAN
    bTableIsScenario = FALSE
'///<ul>
    '///<li>Check correct table name</li>///                           
    if SAXGetAttributeValue( "table:name" ) = sTableName then
        printlog "Table name is as expected"
    else
        warnlog "Table name is " & SAXGetAttributeValue( "table:name" ) & " instead of " & sTableName
    endif
    '///<li>Find out if current table is a scenario</li>///
    try 
        SAXSeekElement( "table:scenario")
        bTableIsScenario = TRUE
        if bTableShouldBeScenario then
            printlog "OK, this table is a scenario!"
        else
            warnlog "OOPS, this table shouldn't be a scenario -> Check this out!"
        endif
    catch
        if bTableShouldBeScenario then
            warnlog "OOPS, this table should be a scenario but it isn't -> Check this out!"
        else
            printlog "Right, this table isn't a scenario"
        endif
    endcatch
    '///<li>Check scenario attributes</li>///
    if bTableShouldBeScenario AND bTableIsScenario then
        if bCheckDisplayBorder then 
            if SAXGetAttributeValue ( "table:display-border" ) = "false" then
                printlog "OK, attribute 'table:display-border' has expected value: 'false'"
            else
                warnlog "OOPS, attribute 'table:display-border' has unexpected value: " & SAXGetAttributeValue( "table:display-border" )
            endif
        else 
            if SAXGetAttributeValue ( "table:display-border" ) <> "" then
                warnlog "OOPS, attribute 'table:display-border' shouldn't exist here"
            endif
        endif

        if SAXGetAttributeValue( "table:border-color" ) = sBorderColor then
            printlog "OK, attribute 'table:border-color' has expected value"
        else
            warnlog "OOPS, attribute 'table:border-color' has unexpected value: " & SAXGetAttributeValue( "table:border-color" )
        endif
        
        if bCheckCopyBack then
            if SAXGetAttributeValue( "table:copy-back" ) = "false" then
                printlog "OK, attribute 'table:copy-back' has expected value: 'false'"
            else
                warnlog "OOPS, attribute 'table:copy-back' has unexpected value: " & SAXGetAttributeValue( "table:copy-back" )
            endif
        else 
            if SAXGetAttributeValue ( "table:copy-back" ) <> "" then
                warnlog "OOPS, attribute 'table:copy-back' shouldn't exist here"
            endif
        endif
        
        if SAXGetAttributeValue( "table:copy-styles" ) = sCopyStyles then
            printlog "OK, attribute 'table:copy-styles' has expected value"
        else
            warnlog "OOPS, attribute 'table:copy-styles' has unexpected value: " & SAXGetAttributeValue( "table:copy-styles" )
        endif
        
        if bCheckProtected then
            if SAXGetAttributeValue( "table:protected" ) = "true" then
                printlog "OK, attribute 'table:protected' has expected value: 'true'"
            else
                warnlog "OOPS, attribute 'table:protected' has unexpected value: " & SAXGetAttributeValue( "table:protected" )
            endif
        else 
            if SAXGetAttributeValue ( "table:protected" ) <> "" then
                warnlog "OOPS, attribute 'table:protected' shouldn't exist here"
            endif
        endif

        if SAXGetAttributeValue( "table:is-active" ) = sIsActive then
            printlog "OK, attribute 'table:is-active' has expected value"
        else
            warnlog "OOPS, attribute 'table:is-active' has unexpected value: " & SAXGetAttributeValue( "table:is-active" )
        endif

        if SAXGetAttributeValue( "table:scenario-ranges" ) = sScenarioRanges then
            printlog "OK, attribute 'table:scenario-ranges' has expected value"
        else
            warnlog "OOPS, attribute 'table:scenario-ranges' has unexpected value: " & SAXGetAttributeValue( "table:scenario-ranges" )
            qaErrorLog "Expected: " & sScenarioRanges
        endif

        if SAXGetAttributeValue( "table:comment" ) = sComment then
            printlog "OK, attribute 'table:comment' has expected value"
        else
            warnlog "OOPS, attribute 'table:comment' has unexpected value: " & SAXGetAttributeValue( "table:comment" )
            qaErrorLog "Expected: " & sComment
        endif
    else
        printlog "Checking scenario details was skipped!"
    endif
    
    if bTableIsScenario then
        SAXSeekElement( 0 )
    endif
'///</ul>
end sub


