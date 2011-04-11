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
'/******************************************************************************
'*
'*  owner : gregor.hartmann@oracle.com
'*
'*  short description : Retrieve and set filternames and suffixes
'*
'\******************************************************************************

private const LENGTH_OF_FILTERFILE = 100
private const FILE_DATA_SIZE = 300

function hCheckForBinfilters() as boolean

    try
        hGetSuffix( "569" )
        hCheckForBinfilters() = true
    catch
        warnlog( "Optional legacy filters package is not installed" )
        printlog( "Please restart the setup to install the missing filters" )
        hCheckForBinfilters() = false
    endcatch

end function

'*******************************************************************************

function hGetSuffix( optional cBuildId as string ) as string

    ' This function retrieves the suffix depending on the build id (e.g. 680)
    ' for a known gApplication from the program configuration.

    ' Currently known Build-IDs are:
    ' No parameter = current
    ' "" (empty string) = current
    ' 300 = StarOffice 9 / OpenOffice.org 3.x
    ' 680 = StarOffice 8 / OpenOffice.org 2.x
    ' 645 = StarOffice 7 / OpenOffice.org 1.x
    ' 641 = StarOffice 6 (XML format)
    ' 569 = StarOffice 5 (Binary format)

    dim sMatchingFile as string
    dim sFilterArray( 100 ) as string
    dim sFilterConfigName as string
    dim sSuffix( 10 ) as string

    if ( IsMissing( cBuildId ) ) then cBuildId = "current"
    if ( cBuildId = "" ) then cBuildId = "current"

    sMatchingFile = gTesttoolPath & "global\input\filters\"
    sMatchingFile = sMatchingFile & "build_to_suffix.txt"
    sMatchingFile = convertpath( sMatchingFile )

    'printlog( "DEBUG: SUFFIX: Build-ID: " & cBuildId )
    hGetDataFileSection( sMatchingFile, sFilterArray(), cBuildId, "", "" )
    sFilterConfigName = hGetValueForKeyAsString( sFilterArray(), gApplication )
    'printlog( "DEBUG: SUFFIX: Config name: " & sFilterConfigName )
    sSuffix() = hGetFilterNameExtension( sFilterConfigName )
    'printlog( "DEBUG: SUFFIX: " & sSuffix( 0 ) )
    hGetSuffix() = "." & sSuffix( 0 )

end function

'*******************************************************************************

function hGetFilter( optional cBuildId as string ) as string

    '///<h1>Get the Filtername for a specified Build-ID</h1>

    ' Currently known Build-IDs are:
    ' No parameter = current
    ' "" (empty string) = current
    ' 300 = StarOffice 9 / OpenOffice.org 3.x
    ' 680 = StarOffice 8 / OpenOffice.org 2.x
    ' 645 = StarOffice 7 / OpenOffice.org 1.x
    ' 641 = StarOffice 6 (XML format)
    ' 569 = StarOffice 5 (Binary format)

    dim clTemp( LENGTH_OF_FILTERFILE ) as string ' cFilterFile is stored here

    dim sMatchingFile as string
    dim sFilterArray( 100 ) as string
    dim sFilterConfigName as string
    dim sFilter as string

    if ( IsMissing( cBuildId ) ) then cBuildId = "current"
    if ( cBuildId = "" ) then cBuildId = "current"

    sMatchingFile = gTesttoolPath & "global\input\filters\"
    sMatchingFile = sMatchingFile & "build_to_filter.txt"
    sMatchingFile = convertpath( sMatchingFile )

    'printlog( "DEBUG: FILTER: Filter-ID: " & cBuildId )
    hGetDataFileSection( sMatchingFile, sFilterArray(), cBuildId, "", "" )
    sFilterConfigName = hGetValueForKeyAsString( sFilterArray(), gApplication )
    'printlog( "DEBUG: FILTER: Config name: " & sFilterConfigName )
    sFilter = hGetUIFilterName( sFilterConfigName )
    'printlog( "DEBUG: FILTER: " & sFilter )
    hGetFilter() = sFilter

end function

'*******************************************************************************

function hSelectUIfilter( cAPIFilter as string ) as boolean

    ' Wrapper for hFindFilterPosition() which also selects the filter
    dim irc as integer

    irc = hFindFilterPosition( cAPIFilter )
    if ( irc > 0 ) then
        DateiTyp.select( irc )
        hSelectUIfilter() = true
    else
        hSelectUIfilter() = false
    endif

end function

'*******************************************************************************

function hFindFilterPosition( cFilter as string ) as integer

    ' This function takes a filter as provided by the office API and tries to find
    ' this filter within the File Save dialogs file type list.
    ' The file types have a suffix appended like " (.odt)" which is not present
    ' in the API's filter name so it is not possible to select the file
    ' type directly and we do not have an exact match either.
    ' To ensure that we not accidentially select the template a bracket is
    ' appended to the string.

    dim iCurrentFilter as integer
    dim cCurrentFilter as string
    dim cUniqueFilter as string
    cUniqueFilter = cFilter & " ("

    const CFN = "global::tools::inc::hFindFilterPosition::"

    for iCurrentFilter = 1 to DateiTyp.getItemCount()

        cCurrentFilter = DateiTyp.getItemText( iCurrentFilter )

        if ( cFilter = cCurrentFilter ) then
            'printlog( CFN & "Exact match - this is a UI filter name, not API" )
            'printlog( CFN & "The filter is at pos. " & iCurrentFilter )
            hFindFilterPosition() = iCurrentFilter
            exit function
        endif

        if ( instr( cCurrentFilter, cUniqueFilter ) > 0 ) then
            'printlog( CFN & "Filter found at pos. " & iCurrentFilter )
            hFindFilterPosition() = iCurrentFilter
            exit function
        endif

    next iCurrentFilter

    warnlog( CFN & "Filter not found: " & cFilter )
    warnlog( CFN & "Refer to global::input:.filters::api_filters.txt for a complete list of available filters" )

    hFindFilterPosition() = 0

end function

'*******************************************************************************

function hGetUIFiltername( vFiltername as string ) as string

    '/// Returns the in the UI used filter name.
    '///+ <b>INPUT</b>: 'internal', language independent filter name from FilterFactory.
    '///+ <u>Examples</u>:<ul><li>hGetUIFiltername(&quot;StarOffice XML (Draw)&quot;) - Draw OOo 1.x/SO6.0/SO7 UI Filtername</li>
    '///+ <li>sUIFiltername = hGetUIFiltername(&quot;StarOffice XML (Impress)&quot;) - Impress OOo 1.x/SO6.0/SO7 UI Filtername</li></ul>
    '/// The 'internal' name can be found in the *.xcu in
    '///+ ..../share/registry/res/en-US/org/openoffice/TypeDetection/Filter.xcu.
    '/// See also: hGetFilternameExtension

    Dim iCurrentFilter as integer
    Dim oOpenUNOService as object
    Dim oFilterName as object
    Dim oUno as object

    const CFN = "global::tools:includes:required::t_filters.inc::hGetUIFiltername(): "

    oUno = hGetUNOService( TRUE )

    oOpenUNOService = oUno.createInstance( "com.sun.star.document.FilterFactory" )
    try
        oFilterName = oOpenUNOService.getByName( vFiltername )

        for iCurrentFilter = 0 to ubound( oFilterName )
            if ( oFilterName( iCurrentFilter ).Name = "UIName" ) then
                hGetUIFiltername = oFilterName( iCurrentFilter ).Value
            end if
        next iCurrentFilter
    catch
        warnlog ( CFN & vFiltername & "'): Filtername is not available." )
        hGetUIFiltername() = ""
    endcatch

end function

'*******************************************************************************

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

    Dim iCurrentExtension as integer
    Dim oOpenUNOService as object
    Dim oFilterNameExtension as object
    Dim oUno as object
    dim aExtensionsSize as integer
    Dim aExtensions() as string

    const CFN = "global::tools:includes:required::t_filters.inc::hGetFilternameExtension(): "

    'Initializize UNO communication
    oUno = hGetUNOService( TRUE )

    'Using the TypeDetection service
    oOpenUNOService = oUno.createInstance("com.sun.star.document.TypeDetection")

    'Getting the Extension by given (internal; language- and product
    'independent) filter name
    oFilterNameExtension = oOpenUNOService.getByName(vFiltername)

    'using ubound to count the nodes
    for iCurrentExtension = 0 to ubound( oFilterNameExtension )

        'if the node name is 'Extensions'...
        if ( oFilterNameExtension( iCurrentExtension ).Name = "Extensions" ) then

            '...if it's an array...
            if ( IsArray( oFilterNameExtension( iCurrentExtension ).Value) ) then

                'set the size of the aExtensions() array
                aExtensionsSize = 10

                're-dimension the array with the integer a
                Redim aExtensions( aExtensionsSize ) as string

                'return the array into an array
                aExtensions() = oFilterNameExtension( iCurrentExtension ).Value()
            else
                '...otherwise 'build' an array with only
                'one entry in (0)
                Redim aExtensions( 0 ) as string
                aExtensions( 0 ) = oFilterNameExtension( iCurrentExtension ).Value
            endif
        endif
    next iCurrentExtension

    'put the results into the return value of this function into an array.
    hGetFilternameExtension = aExtensions()

end function

'*******************************************************************************

function hGetValueForKeyAsString( lsList() as string, sKey as string ) as string

    '/// This function returns the value of a key as string.
    '///+ The form of the input strings is 'key=value', the list is parsed
    '///+ The Value for the first occurrence of sKey is returned

    dim iItem as integer
    dim cComp as string

    hGetValueForKeyAsString() = "Error: No matching VALUE found for key: " & sKey

    ' Scan through the list and look for sKey. If found, return the Value
    ' (everything to the right of the '=')

    for iItem = 1 to listcount( lsList() )

        if( instr( lsList( iItem ) , sKey ) <> 0 ) then

            cComp = hGetKeyforPairAsString( lsList( iItem ) )

            if( sKey = cComp ) then
                hGetValueForKeyAsString() = hGetValueForPairAsString( lsList( iItem ) )
                iItem = listcount( lsList() ) + 1
            end if

        end if

    next iItem

end function

'*******************************************************************************

function hGetValueForPairAsString( cLine as string ) as string

    '///  This function takes a string that (hopefully) contains one '='
    '///+  and returns the substringstring to the right from the '=' char.

    dim iCharPos as integer

    iCharPos = instr( cLine , "=" )
    iCharPos = len( cLine ) - iCharPos
    hGetValueForPairAsString() = right( cLine , iCharPos )

end function

'*******************************************************************************

function hGetKeyForPairAsString( cLine as string ) as string

    '///  This function returns the string to the left of the '='

    dim iCharPos as integer

    iCharPos = instr( cLine , "=" )

    ' get the string to the left of the = char

    if ( iCharPos > 0 ) then
        hGetKeyForPairAsString() = left( cLine , iCharPos -1 )
    else
        warnlog( "Invalid string passed to hGetKeyForPairAsString: " & cLine )
    end if

end function

'*******************************************************************************

function hGetDataFileSection( cFile as string, lsList() as string, cSection as string , cComment as string, cPrint as string ) as integer

    const CFN = "hGetDataFileSection:"

    '///  This function reads a datafile into a list.
    '///+ Comments (lines beginning with #) are removed from the list.
    '///+ A comment can be passed to the log.
    '///+ Furthermore a section in the source-file can be specified. Only
    '///+ lines within the section are returned then. The delimiter for a
    '///+ section is [section-name] <> [ ...] (or EOF)
    '///+ <ul>NOTES:<li>Instead of lsList() a temporary list should be used that is big
    '///+ enough to hold all the data from the datafile.</li>
    '///+ <li> this function does - basically the same as getinivalue(...) but
    '///+ is easier to debug and returns a list not the key.</li></ul>

    dim sFile as string

    dim iSectionBegin as integer
    dim iSectionEnd as integer
    dim iSectionItems as integer

    ' verify that the sourcefile exists, otherwise warn and abort
    if ( dir( cFile ) = "" ) then
        warnlog( CFN & "File not found: " & cFile )
        hGetDataFileSection() = 0
        exit function
    end if

    ' print a comment to the logfile. Non optional parameter but might be ""
    if ( cComment <> "" ) then
        printlog( "" )
        printlog( CFN & cComment & " : " & cFile )
        printlog( "" )
    end if

    ' read the list from the file
    listread( lsList() , cFile , "utf8" )

    ' remove comments ( lines containing # )
    hListClearPattern( lsList() , "#" )

    ' remove all blank lines
    hListClearBlank( lsList() )

    ' honor the section, if given. Non-optional parameter that can be ""
    if ( cSection <> "" ) then
        iSectionBegin = hGetStartOfSection( lsList() , cSection )
        iSectionEnd   = hGetEndOfSection( lsList() , iSectionBegin )
        iSectionItems = hGetSection( lsList() , iSectionBegin , iSectionEnd )
    end if

    ' print the current list - if desired.
    if ( lcase( cPrint ) <> "" ) then
        hListPrint( lsList(), "Parent function: " & CFN )
    end if

    ' return the number of items
    hGetDataFileSection() = listcount( lsList() )

end function

'*******************************************************************************

function hGetFileData( sFile as string , sKey as string ) as string

    dim sList( FILE_DATA_SIZE ) as string
    dim iArraySize as integer

    ' This function reads a file and returns the first line containing sKey
    iArraySize = hGetDataFileSection( sFile, sList(), "", "", "" )
    hGetFileData() = hGetValueForKeyAsString( sList() , sKey )

end function

'*******************************************************************************

function hGetStartOfSection( lsList() as string , _section as string ) as integer

    const CFN = "hGetStartOfSection::"

    '/// This function takes a list and looks for a string of the type [_section].
    '///+ The position of this successful hit is returned.
    '///+ On error the returnvalue defaults to 0.

    dim iThisString as integer
    dim cThisString as string
    dim iListSize as integer
    dim cSection as string

    iThisString = 0
    cSection = lcase ( "[" & _section & "]" )
    iListSize = listcount( lsList() )

    do while ( iThisString <= iListSize )

        iThisString = iThisString + 1
        cThisString = lcase( lsList( iThisString ) )

        if ( instr( cThisString , cSection ) ) then
            hGetStartOfSection() = iThisString + 1
            iThisString = iListSize + 5
        end if

    loop

    if ( iThisString = ( iListSize + 1 ) ) then
        warnlog( CFN & "Section not found or empty: " & _section )
        hGetStartOfSection = 0
    end if

end function

'*******************************************************************************

function hGetEndOfSection( lsList() as string , iOffset as integer ) as integer

    dim iThisString as integer
    dim cThisString as string
    dim iListSize as integer

    iThisString = iOffset
    iListSize = listcount( lsList() )

    do while ( iThisString <= iListSize )
        cThisString = lsList( iThisString )

        if ( ( instr( cThisString , "[" ) > 0 ) and  ( instr( cThisString , "]" ) > 0 ) )then
            hGetEndOfSection() = iThisString - 1
            iThisString = iListSize + 5
        else
            iThisString = iThisString + 1
        end if

    loop

    if ( iThisString = ( iListSize + 1 ) ) then
        hGetEndOfSection() = iListSize
    end if

end function

'*******************************************************************************

function hGetSection( lsList() as string , iSectionBegin as integer , iSectionEnd as integer ) as integer

    dim iArraySize as integer
    dim iThisString as integer

    iArraySize = ubound( lsList() )

    dim lsTempList( iArraySize ) as string
    listcopy( lsList() , lsTempList() )
    listalldelete( lsList() )

    for iThisString = iSectionBegin to iSectionEnd

        listappend( lsList() , lsTempList( iThisString ) )

    next iThisString

    hGetSection() = listcount( lsList() )

end function

'*******************************************************************************

function hGetUsedFilter () as string

    ' used in math and graphics modules
    '/// Get used filter for loaded file.
    try
        FileSaveAs
        Kontext "SpeichernDlg"
        hGetUsedFilter = dateityp.getseltext
        SpeichernDlg.Cancel
    catch
        hGetUsedFilter = "Not possible; try/catch fail in function"
    endcatch
end function

