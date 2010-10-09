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
'* short description : Conversion of Microsoft OLE objects in dependency of settings in Tools::Options::Load/Save::MS_Office
'*
'************************************************************************
'*
' #1 ms_ole        ' Initial routine
' #1 tXML_OLE_Conversion
' #1 subSetConversionMicrosoftOLE
'*
'\************************************************************************

testcase tXML_OLE_Conversion ( sCurrentTestDocument as STRING )
    dim iIndex as INTEGER
    dim sInputFile as STRING
    sInputFile = convertpath ( gTesttoolpath & "xml/optional/input/framework/ms_ole/" & sCurrentTestDocument )    
    dim sSplitCurrentTestDocument() as STRING
    sSplitCurrentTestDocument() = split ( sCurrentTestDocument , "." )
    dim sCurrentFileName as STRING
    sCurrentFileName = sSplitCurrentTestDocument ( 0 )
    dim sCurrentExtension as STRING
    sCurrentExtension = sSplitCurrentTestDocument ( 1 )   
    dim sCurrentOLE as STRING
    sCurrentOLE = left ( sCurrentTestDocument , 3 )
    dim sExtNames( 5 ) AS String
    dim sCurrentExtensionOasis as STRING
    select case sCurrentExtension
           case ( "doc" ) : sExtNames() = hGetFilternameExtension ( "writer8" )
           case ( "xls" ) : sExtNames() = hGetFilternameExtension ( "calc8" )
           case ( "ppt" ) : sExtNames() = hGetFilternameExtension ( "impress8" )
    end select
    sCurrentExtensionOasis = " (." & sExtNames(0) & ")"
    dim sCurrentFilter as STRING
    select case sCurrentExtension
           case ( "doc" ) : sCurrentFilter = "writer8"
           case ( "xls" ) : sCurrentFilter = "calc8"
           case ( "ppt" ) : sCurrentFilter = "impress8"
    end select   
    dim sConversionSettings as STRING  
    dim sStorageName as STRING
    sStorageName = ( gOfficePath & ConvertPath("user\work\xml_") & sCurrentFileName )
    dim sOutputFile as STRING
    dim sUnpackedStorageDir as STRING
    dim sContentXML as STRING
    dim sContentXMLObject as STRING
    dim bContentXMLObjectExists as BOOLEAN
    dim sRelativePathToObject as STRING           
    for iIndex = 1 to 4        
        printlog "Set desired conversion options in Tools::Options::Load/Save::MS Office"
        select case iIndex
               case 1 : sConversionSettings = "RESET"
               case 2 : sConversionSettings = "SAVE"  
               case 3 : sConversionSettings = "LOAD"
               case 4 : sConversionSettings = "ALL"
        end select
        call subSetConversionMicrosoftOLE (  sConversionSettings  )
        sleep(2)
        sOutputFile = sStorageName & "_" & sConversionSettings & "." & sExtNames(0)
        sUnpackedStorageDir = sOutputFile & "_unpacked" 
        'Erase storage dirctory if exists
        if (dir ( sUnpackedStorageDir ) <> "" ) then
            rmDir ( sUnpackedStorageDir )
            if (dir ( sUnpackedStorageDir ) <> "" ) then
                warnlog "Removing directory failed. Exiting test."
                goto endsub
            endif
        endif
        sContentXML = sUnpackedStorageDir & ConvertPath( "\content.xml" )                
        printlog "Open test document (MS format)"
        call hFileOpen(sInputFile)
        printlog Chr(13) & "Loading: " & sInputFile
        sleep(2)        
        printlog "Save document in OpenDocument format"
        if hFileSaveAsWithFilterKill ( sOutputFile , sCurrentFilter ) = FALSE then
            warnlog "Saving the testdocument failed!"
        endif
        sleep(2)        
        printlog "Close document"
        call hCloseDocument                
        sleep(2)                
        printlog "Unpack storage to obtain the XML files"        
        try 
            UnpackStorage( sOutputFile , sUnpackedStorageDir )
        catch
            qaErrorLog "#i52632# - Unpacking XML storage failed -> we quit this iteration"
            goto skipThisIteration 'Quit only current Iteration (not testcase) on error
        endcatch                
        printlog "Read DOM of content.xml"
        SAXReadFile( sContentXML )
        sleep(2)        
        printlog "Set the DOM pointer on element 'office:body'"        
        SAXSeekElement( "/" )
        SAXSeekElement( "office:document-content" )
        SAXSeekElement( "office:body" )        
        printlog "Use recursion to find XML element 'draw:object-ole' or'draw:object'"
        select case sConversionSettings
               case "RESET" , "SAVE"
                    if hXMLSeekElementInTree ( "draw:object-ole" ) = FALSE then
                        warnlog "Not finding the node 'draw:object-ole' is unexpected -> Check this out!"
                        SAXRelease
                        goto skipThisIteration 'Quit only current Iteration (not testcase) on error
                    endif
               case "LOAD" , "ALL"
                    if hXMLSeekElementInTree ( "draw:object" ) = FALSE then
                        warnlog "Not finding the node 'draw:object' is unexpected -> Check this out!"
                        SAXRelease
                        goto skipThisIteration 'Quit only current Iteration (not testcase) on error
                    endif
        end select        
        sRelativePathToObject = SAXGetAttributeValue( "xlink:href" )
        sRelativePathToObject = Right ( sRelativePathToObject , Len ( sRelativePathToObject ) - 1 )
        sContentXMLObject = sUnpackedStorageDir & ConvertPath ( sRelativePathToObject & "\content.xml" )
        printlog "Check existence of objects 'content.xml' in dependency of settings in Tools::Options::Load/Save::MS_Office"        
        printlog "Lookup if objects 'content.xml' exists - TRUE for OASIS OLE, FALSE for MS OLE"
        if (dir ( sContentXMLObject ) <> "" ) then
            bContentXMLObjectExists = TRUE
        else
            bContentXMLObjectExists = FALSE
        endif        
        printlog "Verify result against expectations for existence of objects 'content.xml'"
        select case sConversionSettings
               case "RESET" , "SAVE"
                    if bContentXMLObjectExists then
                        warnlog "+ OOPS, 'content.xml' shouldn't exist in MS OLE directory -> Check this out!" 
                    endif
               case "LOAD" , "ALL"
                    if bContentXMLObjectExists = FALSE then
                        warnlog "+ OOPS, 'content.xml' must exist in OASIS OLE directory -> Check this out!"
                    endif
        end select        
        printlog "Release documents DOM"
        SAXRelease
        if bContentXMLObjectExists then
            printlog "Read the OLE object DOM."            
            SAXReadFile( sContentXMLObject )
            sleep(2)            
            printlog "Check if expected OLE object was found in 'content.xml'"
            SAXSeekElement( "/" )
            select case sCurrentOLE
                   case "mat"
                        SAXSeekElement( 1 )
                        if SAXGetElementName() <> "math" then
                            warnlog "A Math object was expected here -> Check this out!"
                        endif
                   case else
                        SAXSeekElement( "office:document-content" )
                        SAXSeekElement( "office:body" )
                        SAXSeekElement( 1 )
                        select case sCurrentOLE
                               case "exc"
                                    if SAXGetElementName() <> "office:spreadsheet" then
                                        warnlog "+ OOPS, this OLE seems to be no spreadsheet -> Check this out!"
                                    endif
                               case "ppt"
                                    if SAXGetElementName() <> "office:presentation" then
                                        warnlog "+ OOPS, this OLE seems to be no presentation -> Check this out!"
                                    endif
                               case "wor"
                                    if SAXGetElementName() <> "office:text" then
                                        warnlog "+ OOPS, this OLE seems to be no text -> Check this out!"
                                    endif
                        end select
            end select
            SAXRelease
        endif
     skipThisIteration:
     next iIndex
endcase
'
'-----------------------------------------------------------------------------
'
sub subSetConversionMicrosoftOLE ( OPTIONAL sConversionType as STRING )
'Routine to adjust the behaviour of Tools::Options::Load/Save::Microsoft Office
'Possible Keys for 'sConversionType' are:
'RESET (or omitted): Uncheck all boxes (default in a freshly installed OOo))
'ALL: Check all boxes 
'LOAD: Check all boxes for loading MS OLE objects (left side), uncheck all boxes for saving
'SAVE: Check all boxes for saving MS OLE objects (right side), uncheck all boxes for loading
'For other values the subroutine will exit

    if IsMissing ( sConversionType ) then
       sConversionType = "RESET"
    end if
    dim iPosInAuswahl as INTEGER
    dim iExtraToggle as INTEGER
    
    'Determine value of 'iExtraToggle' for known keys
    select case sConversionType
           case "RESET" : iExtraToggle = 1
           case "ALL"   : iExtraToggle = 2
           case "LOAD"  : iExtraToggle = 3
           case "SAVE"  : iExtraToggle = 4
           case else 
                warnlog "Unknown keyword for paramter -> Exit the subroutine"
                qaErrorLog "Valid keywords are RESET, ALL, LOAD and SAVE!"
                exit sub
    end select
    
    printlog "Invoke Tools::Options"       
    ToolsOptions
    printlog "Go to tabpage Load/Save::Microsoft Office"
    call hToolsOptions ( "LOADSAVE" , "MICROSOFTOFFICE" )    
    printlog "Go to top entry in listbox"
    Auswahl.TypeKeys( "<PageUp>" )        
    printlog "Toggle (in a tricky way) to a defined state of checkboxes"
    for iPosInAuswahl = 1 to 4
         Auswahl.TypeKeys ( "<RIGHT>" , 2 )
         while NOT Auswahl.IsChecked
             Auswahl.TypeKeys ( "<Space>" )
         wend
         while Auswahl.IsChecked
             Auswahl.TypeKeys ( "<Space>" )
         wend         
         printlog "Toggle to desired state of boxes"
         Auswahl.TypeKeys ( "<Space>" , iExtraToggle )         
         printlog "Go one entry down in list box"
         Auswahl.TypeKeys ( "<Down>" )         
         printlog "Loop for all lines"
    next iPosInAuswahl        
    printlog "Quit dialog"
    Kontext "OptionenDLG"
    OptionenDLG.Ok    
end sub

