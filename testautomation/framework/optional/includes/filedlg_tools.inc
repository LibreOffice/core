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
'* owner : gregor.hartmann@oracle.com
'*
'* short description : check the internal file dialog ( 1. part )
'*
'\***************************************************************************

sub CheckDirectoryName( dirname as string )
    
    ' Try to create a folder twice and make sure a warning comes up that the
    ' folder already exists
    
    dim i as integer
    dim FULLPATH as string
    FULLPATH = gOfficePath + "user\work\" + dirname + "\"
    
    printlog( " - Create the folder and verify it's existence" )
    
    try
        
        for i=1 to 2
            
            printlog( " - creating folder for the " + i + ". time" )
            
            Kontext "OeffnenDlg"
            printlog( " - press 'new folder' button" )
            NeuerOrdner.Click()
            
            Kontext "NeuerOrdner"
            if ( NeuerOrdner.exists( 2 ) ) then
                printlog( " - name the folder" )
                OrdnerName.SetText( dirname )
                printlog( " - press OK")
                NeuerOrdner.OK()
            else
                warnlog( "New Folder dialog not displayed" )
            endif
            
            ' Take care of the File Exists Dialog
            printlog( " - check for 'File Exists'-Dialog" )
            Kontext "Active"
            If Active.Exists() then
                Active.OK()
                
                'if the file exists during first run of the test: just go on
                if i =  1 then
                    warnlog( "File exists. Did you clean the output-dir?" )
                end if
                
                'if the active exists, we will return to the 'new folder' dialog
                'it must be closed as well
                Kontext "NeuerOrdner"
                NeuerOrdner.Cancel()
            else
                'this should only be displayed on second run
                if i = 2 then
                    warnlog( "No message that the folder '" + dirname + "' exists!" )
                end if
            end if
            
        next i
        
        printlog( " - verify the existence of the new directory" )
        if App.Dir ( ConvertPath ( FULLPATH ), 16 ) = "" then
            Warnlog( "The directory'" + dirname + "' wasn't created!" )
        else
            printlog( " - delete it")
            App.RmDir ( ConvertPath ( FULLPATH ) )
        end if
        
        
    catch
        
        warnlog( "Could not create the directory. Bugid: #108256# or #106510# ?" )
        
    endcatch
    
end sub

'*******************************************************************************

sub CreateInvalidDirectory( dirname as string )
    
    'Try to create a directory with a name that is invalid by using characters
    'that are not allowed for a filesystems. An errormessage is expected.
    
    dim i as integer
    dim FULLPATH as string
    FULLPATH = gOfficePath + "user\work\" + dirname + "\" 'experimental
    
    printlog( " - Trying to create directory <" + FULLPATH + ">" )
    
    try
        
        Kontext "OeffnenDlg"
        if ( OeffnenDlg.exists( 2 ) ) then
            
            NeuerOrdner.Click()
            
            Kontext "NeuerOrdner"
            if ( NeuerOrdner.exists( 2 ) ) then
                
                OrdnerName.SetText( dirname )
                NeuerOrdner.OK()
            else
                warnlog( "New Folder dialog did not open" )
            endif
            
            
            if gPlatgroup() = "unx" then
                
                'unx allows weird filenames
                if App.Dir ( ConvertPath ( FULLPATH ), 16 ) = "" then
                    Warnlog "  the '" + dirname + "'-dir wasn't created!"
                else
                    App.RmDir ( ConvertPath ( FULLPATH ) )
                end if
                
            else
                
                'windows
                Kontext "Active"
                if Active.Exists( 2 ) then
                    
                    printlog( " - handle the 'Active' dialog" )
                    Active.OK()
                    
                    Kontext "Active"
                    if Active.Exists( 2 ) then
                        warnlog( "Double errormessage displayed, one expected." )
                        Active.OK()
                    end if
                    
                    Kontext "NeuerOrdner"
                    NeuerOrdner.Cancel()
                    
                else
                    
                    warnlog( "No warning that the folder can't be created!" )
                    
                end if
                
                if App.Dir( ConvertPath ( FULLPATH ), 16 ) <> "" then
                    Warnlog "The directory '"+ dirname + "' contains invalid chars"
                    App.RmDir( ConvertPath ( FULLPATH ) )
                end if
                
            end if
            
        endif
            
    catch
        
        warnlog( "Could not create the directory. Bugid: #108256# or #106510# ?" )
        
    endcatch
        
end sub

'*******************************************************************************

sub CreateValidDirectory( dirname as string )
    
    'Create a directory that has a valid name at a valid location
    'The directory should not exist.
    'It will be created in the user's directory
    
    dim FULLPATH as string
    FULLPATH = gOfficePath() + "user\work\" + dirname + "\" 'experimental
    
    
    printlog( " - Trying to create directory: <" + FULLPATH + ">" )
    
    try
        
        printlog( " - Open the file-open dialog" )
        Kontext "OeffnenDlg"
        NeuerOrdner.Click()
        
        printlog( " - name the folder" )
        Kontext "NeuerOrdner"
        OrdnerName.SetText( dirname )
        NeuerOrdner.OK()
        
        printlog( " - check if the directory has been created" )
        if App.Dir ( ConvertPath ( FULLPATH ) , 16 ) = "" then
            warnlog( "The '" + dirname + "' has not been created!" )
        else
            App.RmDir( ConvertPath ( FULLPATH ) )
            printlog( " - existing file was successfully deleted" )
        end if
        
    catch
        
        warnlog( "Could not create the directory. Bugid: #108256# or #106510# ?" )
        
    endcatch
    
end sub

'*******************************************************************************

sub CreateValidDirectoryCrop( dirname as string , cropname as string)
    
    'Create a directory that has a name with leading or trailing spaces.
    'Those names are valid but must be cropped.
    'The directory should not exist.
    'It will be created in the user's directory.
    
    dim FULLPATH as string
    FULLPATH = gOfficePath + "user\work\" + dirname + "\" 'experimental
    dim CROPPATH as string
    CROPPATH = gOfficePath + "user\work\" + cropname + "\" 'experimental
    
    printlog( " - Trying to create directory: <" + FULLPATH + ">" )
    
    try
        
        printlog( " - open the file-open dialog" )
        Kontext "OeffnenDlg"
        NeuerOrdner.Click()
        
        printlog( " - name the folder" )
        Kontext "NeuerOrdner"
        OrdnerName.SetText( dirname )
        NeuerOrdner.OK()

        
        printlog( " - check if the directory has been created" )
        if App.Dir( ConvertPath ( CROPPATH ), 16 ) = "" then
            
            warnlog( "The leading or trailing spaces have not been deleted!" )
            
        else
            

            App.RmDir( ConvertPath ( CROPPATH ) )
            printlog( " - existing directory was successfully deleted." )
            
        end if
        
    catch
        
        warnlog( "Could not create the directory. Bugid: #108256# or #106510# ?" )
        
    endcatch
    
end sub

'*******************************************************************************

function LoadDocumentReadOnly ( sDatei as String, TypeOfDocument as Integer, ReadOnlyFlag as Boolean ) as Boolean
    
    LoadDocumentReadOnly = TRUE
    
    FileOpen
    
    Kontext "OeffnenDlg"
    Dateiname.SetText sDatei
    
    if ReadOnlyFlag = TRUE then
        NurLesen.Check()
    endif
    
    Oeffnen.Click
    hFileWait( FALSE )
    
    Kontext "Active"
    if Active.Exists( 2 ) then
        LoadDocumentReadOnly = FALSE
        Warnlog Active.GetText
        Active.OK

        if Active.Exists( 1 ) then
            Warnlog "A second messagebox is active!"
            Active.OK
        end if
        
        Kontext "Filterauswahl"
        If FilterAuswahl.Exists( 2 ) then
            Warnlog "Filterbox is also active!"
            FilterAuswahl.Cancel
        end if
    end if
    
    'printlog( " - check for (unexpected) filter-dialog" )
    Kontext "Filterauswahl"
    
    If ( FilterAuswahl.Exists( 2 ) ) then
        Warnlog( "Filterbox is active, this is not expected" )
        FilterAuswahl.Cancel()
        
        qaerrorlog( "TODO: There is no way that this function can handle " & _
        "the filterdialog correctly, rewrite it!" )
        warnlog( "The test will most likely fail!" )
        
        LoadDocumentReadOnly() = false
        exit function
        
    endif
    
    
    
    kontext "OeffnenDlg"
    
    if ( OeffnenDlg.exists( 2 ) ) then
        warnlog( "File-Open dialog is open, this is not expected at this point" )
        printlog( "Exiting routine to recover" )
        
        OeffnenDlg.cancel()
        
        LoadDocumentReadOnly() = false
        exit function
        
    endif
    
    try
        
        select case TypeOfDocument
            
        case 1  : InsertSection                           ' Writer
            Kontext
            Active.Setpage TabBereiche
            Kontext "TabBereiche"
            TabBereiche.Cancel
        case 2  : FormatCells                             ' Calc
            Kontext
            Active.SetPage TabZahlen
            Kontext "TabZahlen"
            TabZahlen.Cancel
        case 3  : SlideShowPresentationSettings           ' Impress
            Kontext "Bildschirmpraesentation"
            Bildschirmpraesentation.Cancel
        case 4  : InsertLayer                             ' Draw
            Kontext "EbeneEinfuegenDlg"
            EbeneEinfuegenDlg.Cancel
        case 5  : FormatLegend                            ' Chart
            Kontext "TabUmrandungChart" :
            TabUmrandungChart.Cancel
        case 6  : FormatFonts                             ' Math
            Kontext "Schriftarten"
            Schriftarten.Cancel
        end select
        
        if LoadDocumentReadOnly = TRUE then
            Warnlog "The document wasn't loaded read only"
        endif
        
    catch
        
    endcatch
    
end function

'*******************************************************************************

sub hSetUNIXAttributes()
    
    ' NOTE: Requested by TBO
    ' This sub tries to set the file-attributes on the qatesttool-snapshot on
    ' mahler.germany.
    ' Since all BASIC commands are platform independent, it cannot be checked
    ' exactly.
    ' If you are not the owner of the testtool snapshot, this is *not* going to
    ' work.
    
    
    dim cFullPath as string
    dim cParameter as string
    dim cCommand as string
    dim rc as integer
    
    if ( gPLatGroup = "unx" ) then
        
        cCommand = "chmod" ' Fallback, if no following definition matches
        if ( gPlatform = "sol" ) then
            cCommand = "/usr/bin/chmod"
        endif
        if ( gPlatform = "x86" ) then
            cCommand = "/usr/bin/chmod"
        endif
        if ( gPlatform = "lin" ) then
            cCommand = "/bin/chmod"
        endif
        
        cFullPath = convertpath( gTesttoolPath + "framework/filedlg/input/noentry" )
        printlog( "Trying to set attributes for "  + cFullPath )
        cParameter = "-R 000 " + cFullPath
        rc = shell( cCommand , 0 , cParameter , false )
        
        ' the readentry-directory must be readonly, the files as well
        cFullPath = convertpath( gTesttoolPath + "framework/filedlg/input/readentry" )
        printlog( "Trying to set attributes for " + cFullPath )
        cParameter = "-R 444 " + cFullPath
        rc = shell( cCommand , 0 , cParameter , false )
        'need to remove the "S" attribute
        cParameter = "-R -s " + cFullPath
        rc = shell( cCommand , 0 , cParameter , false )
        
        ' the readonly-directory: Only the directory is read-only, the files are rw
        cFullPath = convertpath( gTesttoolPath + "framework/filedlg/input/readonly/*.*" )
        printlog( "Trying to set attributes for " + cFullPath )
        cParameter = "444 " + cFullPath
        rc = shell( cCommand , 0 , cParameter , false )
        
    else
        
        printlog( "No file-attribute setting for non-UNIX platforms" )
        
    endif
    
end sub


'*******************************************************************************

function hGetFirstNameFromFileList() as string
    
    
    
    
    
    
    const CFN = "hGetFirstNameFromFileList()::"
    dim brc as boolean 'a multi purpose boolean returnvalue
    dim cFileName as string
    
    
    Kontext "OeffnenDlg"
    Dateiauswahl.typeKeys( "<HOME>" )
    Dateiauswahl.typeKeys( "<SPACE>" )
    
    cFileName =lcase( Dateiauswahl.GetSelText() )
    printlog( CFN & "File: " & cFileName )
    
    if ( cFileName = "" ) then
        warnlog( CFN & "Filename is empty, probably a timing issue" )
    endif
    
    if ( cFileName = "cvs" ) then
        printlog( CFN & "Skipping CVS directory" )
        Dateiauswahl.TypeKeys( "<Down>" )
        
        cFileName = lcase ( Dateiauswahl.GetSelText() )
        printlog( CFN & "File: " & cFileName )
    endif
    
    hGetFirstNameFromFileList() = cFileName
    
    
end function

