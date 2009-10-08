'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: t_filetools.inc,v $
'*
'* $Revision: 1.4 $
'*
'* last change: $Author: rt $ $Date: 2008-09-04 09:17:26 $
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
'*  owner : joerg.skottke@sun.com
'*
'*  short description : Functions that work with files like deleting, loading...
'*
'\******************************************************************************

Option Explicit

private const CBUILDID = "680" ' This corresponds to the 680 codeline

function hDeleteFile( cFileOrig as string ) as boolean

    const CFN = "hDeleteFile::"
    dim cHome as string
        cHome = gOfficePath & "user"
        cHome = convertpath( cHome )
    dim cFile as string
        cFile = convertpath( cFileOrig )

    '///<h3>Delete a file</h3>
    '///<i>In many cases it is a good idea to use this function outside the testcase.<br>
    '///+ You should always evaluate the returncode</i><br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Filename (string)</li>
    '///<ul>
    '///+<li>The function converts the path to system specific syntax (convertpath)
    '///+ but does <b>not</b> modify the input parameter</li>
    '///</ul>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Errorstatus (boolean)</li>
    '///<ul>
    '///+<li>TRUE: File was deleted/does not exist</li>
    '///+<li>FALSE: File could not be deleted/any other error</li>
    '///</ul>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>
    

    ' this function tries to delete a file and does some very basic error-
    ' handling. Returns 'true' on success, only error while deleting returns
    ' 'false', if the file does not exist, it is considered to be successfully
    ' deleted.
    ' i introduced this function due to a number of cases where deleting files
    ' actually failed because of weird code or situations where the user lacks
    ' accessrights to files are not handled at all.
    
    '///+<li>We may never delete a file outside gOfficePath/user/work</li>
    if ( instr( cFile , cHome ) = 0 ) then
        qaerrorlog( CFN & "Trying to delete file outside (default)homedir -> forbidden" )
        qaerrorlog( CFN & "Home: " & cHome )
        qaerrorlog( CFN & "File: " & cFile )
        hDeleteFile() = false
        exit function
    endif
    
    cFile = convertpath( cFile )

    '///+<li>Check that the file exists</li>
    if ( FileExists( cFile ) ) then

        '///+<li>Use kill to delete</li>
        try

            kill( cFile )

            '///+<li>Verify that the file does not exist anymore</li>
            if ( FileExists( cFile ) ) then
                warnlog( CFN & "File was not deleted: " & cFile )
                hDeleteFile() = false
            else
                printlog( CFN & "File successfully deleted: " & cFile )
                hDeleteFile() = true
            endif

        catch

            '///+<li>in very rare cases 'kill' fails and can be handled this way</li>
            qaerrorlog( CFN & "Deleting file failed: " & cFile )
            hDeleteFile() = false

        endcatch

    else

        '///+<li>write some text to the log if the file does not exist</li>
        printlog( CFN & "Nothing to do." )
        hDeleteFile() = true

    endif
    
    '///</ul>

end function

'*******************************************************************************

function hHandleActivesOnLoad( iTries as integer , iAdd as integer ) as boolean

    '///<h3>Handle any dialogs that might pop up when loading a file</h3>
    '///<i>Beware: This function gives only limited control over the dialogs closed,
    '///+ it just closes anything it can.</i><br>
    '///<i>Please refer to the inline documentation for further details</i><br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Number of dialogs to be closed (integer)</li>
    '///<ul>
    '///+<li>Number of dialogs not limited though more than 3 is not useful</li>
    '///</ul>
    '///+<li>Additional dialogs (integer).</li>
    '///<ul>
    '///+<li>Number of dialogs to handle in case of unexpected behavior. If unsure set it to 2</li>
    '///</ul>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Errorcondition (boolean)</li>
    '///<ul>
    '///+<li>TRUE: The expected number of dialogs were closed</li>
    '///+<li>FALSE: On any other condition</li>
    '///</ul>
     '///</ol>
    '///<u>Description</u>:
    '///<ul>
    '///+<li>OK to create a new document based on a template</li>
    '///+<li>YES to update links</li>
    '///+<li>YES to execute macros</li>
    '///</ul>
    
    use "global/tools/includes/optional/t_stringtools.inc"
    
    const CFN = "hHandleActivesOnLoad::"
    
    dim iTry as integer
    dim iActives as integer
    dim cMessage as string
    dim bSkip as boolean

    printlog( CFN & "Enter" )

    ' not good but apparently required - else hDestroyDocument will handle the
    ' dialogs and warn about them.

    ' think positive: preset the returnvalue with "true"
    hHandleActivesOnLoad() = true
    
    for iTry = 1 to iTries + iAdd

        ' handle three possible dialogs:
        ' 1. The warning that the user should create a new document based
        '    on this Sample/Template (the document is write protected)
        ' 2. The question to update links to the internet. Note: If you
        '    use proxies, they must be set correctly otherwise the result
        '    is undefined.
        ' 3. Some weird usage information dialog which is an active with just
        '    one OK button.

        ' Although this works quite well, there is a flaw that we will not
        ' get information about an "active" dialog that cannot be closed by
        ' .yes() or .ok(). If this happens, we probably run into an error 
        kontext "Active"
        try
            if ( active.exists( 1 ) ) then
               
                iActives = iActives + 1
                cMessage = hRemoveLineBreaks( active.getText() )
                printlog( CFN & "MSG (" & iActives & "): " & cMessage )
                printlog( CFN & "Ressource type is: " & Active.getRT() )
                printlog( CFN & "Buttoncount is: " & Active.getButtonCount() ) 
                bSkip = false

                ' ok to create a new document
                try
                    active.cancel()
                    printlog( CFN & "MSG (" & iActives & "): closed with Cancel" )
                    bSkip = true
                catch
                    printlog( CFN & "Missed - this dialog has no OK button (create new document)" )
                    active.ok()
                endcatch

                if ( not bSkip ) then                
                    if ( Active.getRT() = 304 ) then
                        if ( Active.getButtonCount() = 1 ) then
                            active.ok()
                            printlog( CFN & "MSG (" & iActives & "): closed with OK" )
                            bSkip = true
                        else
                        endif
                    else
                    endif
                endif                    

                if ( not bSkip ) then
                    ' no to update links
                    try
                        active.no()
                        printlog( CFN & "MSG (" & iActives & "): closed with NO" )
                    catch
                        printlog( CFN & "Missed - this dialog has no NO button (update links)" )
                    endcatch
                endif

            endif
        catch
            qaerrorlog( "Problem while trying to handle messageboxes" )
            hHandleActivesOnLoad() = false
        endcatch
        
    next iTry

    ' now see how many dialogs were allowed and how many have been closed
    ' this does not change the return value of the function
    if ( iActives > iTries ) then
        printlog( CFN & "Exit: The test closed more dialogs than expected" )
        hHandleActivesOnLoad() = false
    else
        printlog( CFN & "Exit" )
    endif
       
end function       

'*******************************************************************************

function hHandleInitialDialogs() as integer

    const CFN = "hHandleInitialDialogs::"

    '///<h3>Handle dialogs while opening samples/templates etc.</h3>
    '///<i>The returnvalue is of limited use</i><br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Nothing</li>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Id of the dialog (integer)</li>
    '///<ul>
    '///+<li>0: No dialog was found</li>
    '///+<li>1: UseOfThisTemplate</li>
    '///+<li>2: StarOfficeCalendar1</li>
    '///+<li>3: NewsletterLayout</li>
    '///+<li>4: PortfolioCurrency</li>
    '///+<li>5: StarOfficeCalendar2</li>
    '///</ul>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>
    
    
    ' NOTE: The ID of the dialog will be returned but is of limited use
    '       Function returns 0 if no dialogs have been encountered
    
    dim incident as integer
        incident = 0
        
    printlog( CFN & "Enter" )
    try

        '///+<li>Test for UseOfThisTemplate-dialog</li>
        Kontext "UseOfThisTemplate"
        if ( UseOfThisTemplate.Exists() ) then
            printlog( CFN & "Closing UseOfThisTemplate-dialog" )
            incident = 1
            CancelButton.Click()
        endif
          
        '///+<li>Test for StarOfficeCalendar1-dialog</li>
        Kontext "StarOfficeCalendar1"
        if ( StarOfficeCalendar1.Exists() ) then
            printlog( CFN & "Closing StarOfficeCalendar1-dialog" )
            incident = 2
            Create.Click()
        endif

        '///+<li>Test for StarOfficeCalendar2-dialog (This should be idential to StarofficeCalendar1)</li>
        Kontext "StarOfficeCalendar2"
        if ( StarOfficeCalendar2.Exists() ) then
            printlog( CFN & "Closing StarOfficeCalendar2-dialog" )
            incident = 5
            Create.Click()
        endif
          
        '///+<li>Test for NewsletterLayout-dialog</li>
        Kontext "NewsletterLayout"
        if ( NewsletterLayout.Exists() ) then
            printlog( CFN & "Closing NewsletterLayout-dialog" )
            incident = 3
            OkButton.Click()
        endif
          
        '///+<li>Test for PortfolioCurrency-dialog</li>
        Kontext "PortfolioCurrency"
        if ( PortfolioCurrency.Exists() ) then
            printlog( CFN & "Closing PortfolioCurrency-dialog" )
            incident = 4
            OKButton.Click()
        endif
    catch
    endcatch
         
    hHandleInitialDialogs() = incident
    printlog( CFN & "Exit" )
    
    '///</ul>

end function         

'*******************************************************************************

function hWaitWhileLoading() as boolean

    '///<h3>Wait for a document to load</h3>
    '///<i>Extends: IsItLoaded with 10 retries</i><br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Nothing</li>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Errorstatus (boolean)</li>
    '///<ul>
    '///+<li>TRUE: Document was loaded</li>
    '///+<li>FALSE: Document not loaded</li>
    '///</ul>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>
    
    ' IsItLoaded wastes at least 5 seconds on each call which is bad
    ' for the extras test which run at least 20 minutes longer than 
    ' necessary. The function works the same way as IsItLoaded but
    ' eliminates a lot of sleep-cycles.
    
    dim iNextProbe as integer
    dim brc as boolean
    dim iTime as integer

    const CFN = "hWaitWhileLoading::"
    const I_MAX_WAIT = 30
    const I_PROBE_INTERVAL = 100
    
    printlog( CFN & "Enter" )

    '///+<li>Cycle for at most ten times to check the IsDocLoading-slot</li>
    for iNextProbe = 1 to I_MAX_WAIT
    
        brc = false
        wait( I_PROBE_INTERVAL )

        '///+<li>while the file is loaded the slot will not be dispatched -> catch</li>
        try
            brc = IsDocLoading
            hWaitWhileLoading() = true
            iTime = iNextProbe * I_PROBE_INTERVAL
            printlog( CFN & "File loaded in less than " & iTime & " ms" )
        catch
        endcatch

        '///+<li>"exit for" within try...catch does not always work</li>
        if ( brc ) then
            exit for
        endif

    next iNextProbe

    '///+<li>Warn if the document did not respond in at most 10 seconds</li>
    if ( not brc ) then
        printlog( CFN & "Exit: Did not get feedback within 10 seconds" )
        hWaitWhileLoading() = true
    else
        printlog( CFN & "Exit" )
    endif
    '///</ul>

end function

'*******************************************************************************

function hGetWorkPath() as string

    '///<h3>Retrieve the users work directory</h3>
    '///<i>Uses: Call to UNO service</i><br>
    '///<i>Errorhandling: Fixes &quot;$(user)&quot;-type path (uses fallback)</i><br>
    '///<i>Errorhandling: Handles broken UNO connection (uses fallback)</i><br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Nothing</li>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Path to local workdir (string)</li>
    '///<ul>
    '///+<li>Includes trailing slash/backslash</li>
    '///</ul>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>

    dim sPath as string
    dim sDefaultPath as string
        sDefaultPath = convertpath( gOfficePath & "user/work" )
    dim oOfficeConnect as object
    dim oOfficeConfig as object
    dim bPathIsFromAPI as boolean

    const CFN     = "hGetWorkPath::"

    '///+<li>Create an UNO service and ask it for the location of &quot;Work&quot;</li>
    try
        oOfficeConnect = hGetUnoService( true )
        oOfficeConfig  = oOfficeConnect.createInstance( "com.sun.star.util.PathSettings" )
        sPath = convertFromURL( oOfficeConfig.Work ) 
        bPathIsFromAPI = true
    catch
        '///+<li>Handle broken UNO connection</li>
        printlog( CFN & "Could not access service, connection broken?" )
        sPath = ""
        bPathIsFromAPI = false
    endcatch

    '///+<li>If the path differs from the default do a warnlog</li>
    if ( sPath <> sDefaultPath ) then
        printlog( CFN & "Default path is: " & sDefaultPath )
        printlog( CFN & "Workpath is not defaultpath: " & sPath )
        printlog( CFN & "Overriding UNO path" )
        sPath = convertpath( gOfficePath & "user\work" )
        bPathIsFromAPI = false
    endif
                
    '///+<li>Apply fallback in case of broken connection or invalid path</li>
    if ( instr( sPath , "$(user)" ) > 0 ) then
        printlog( CFN & "sPath has $(user)-type string, using fallback" )
        sPath = sDefaultPath
        bPathIsFromAPI = false
    endif

    '///+<li>Add trailing pathseparator, do convertpath</li>
    sPath = sPath & "/"
    sPath = convertpath( sPath ) 
    
    '///+<li>Print info to the log and return the path</li>
    if ( bPathIsFromAPI ) then
    	printlog( CFN & "(From API): " & sPath )
    else
        qaerrorlog( CFN & "(Junked API-Path): " & sPath )
    endif
    hGetWorkPath() = sPath 
    '///</ul>

end function

'*******************************************************************************

function hGetInputPath( byval sBasePath as string, byval sFile as string ) as string

    '///<h3>Create an input path for reference files</h3>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>BasePath (string) without trailing slash/backslash</li>
    '///<ul>
    '///+<li>Format: gTesttoolPath &amp; &quot;[project]\input\[dir]&quot;</li>
    '///</ul>
    '///+<li>Filename (string)</li>
    '///<ul>
    '///+<li>The name of the file below BasePath to be used</li>
    '///+<li>The filename may be an empty string</li>
    '///</ul>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Full path to file: BasePath + Build-Id + ProductName + sFile</li>
    '///<ul>
    '///+<li>Build-ID (e.g. &quot;680&quot;, from environment)</li>
    '///+<li>ProducName (e.g &quot;OpenOffice.org&quot;, from environment)</li>
    '///</ul>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>
    
    const CFN = "hGetInputPath::"
    dim sPath as string
    dim iAttr as integer
    dim cLastChar as string
    dim iLen as integer
    
    sPath = convertpath( sBasePath )
    
    '///+<li>None of the two parameters may be empty</li>
    if ( sPath = "" ) then
        warnlog( CFN & "Invalid parameter: Empty string passed to function" )
        hGetInputPath() = ""
        exit function
    endif

    '///+<li>Remove trailing slash or backslash</li>
    iLen = len( sPath )
    cLastChar = right( sPath , 1 )
    if ( cLastChar = gPathSigne ) then
        sPath = mid( sPath , 1 , iLen - 1 )
        printlog( CFN & "Truncated path: Trailing Pathseparator not allowed" )
    endif

    '///+<li>At least Basepath should exist</li>
    if ( dir ( sPath ) = "" ) then
        warnlog( CFN & "BasePath does not exist: " & sPath )
        hGetInputPath() = ""
        exit function
    endif    
    
    '///+<li>We must be certain, that basepath is a directory</li>
    iAttr = getAttr( sPath )
    if ( iAttr < 16 or iAttr > 17 ) then
        warnlog( CFN & "Invalid Path: Not a directory: " & sPath )
        hGetInputPath() = ""
        exit function
    endif
     
    '///+<li>Now we concatenate all the information we have to a fully qualified path</li>
    sPath = sPath & "\" & CBUILDID & "\" & gProductName & "\" & sFile
    sPath = convertpath( sPath )
    
    '///+<li>...and return it (without further verification)</li>
    printlog( CFN & sPath )
    hGetInputPath() = sPath
    
    '///</ul>
    
end function

'*******************************************************************************

function hGetFileSizeAsLong( cFileName as string ) as long

    '///<h3>Get the size (long) of a file</h3>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Filename (string)</li>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Size of file (long)</li>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>    
    
    const CFN = "hGetFileSizeAsLong::"

   ' This function returns the size of a specified file. If it is a directory,
   ' the size will default to -1 (A filesize can never be <0 - hopefully)

   dim iAttrib as integer

   '///+<li>Verify that the given file exists</li>
   if ( app.dir( cFileName ) <> "" ) then

      try
          iAttrib = getattr( cFileName )
      
          '///+<li>Return the size for a file or -1 for a directory</li>
          if ( iAttrib <> 16 and iAttrib <> 17 ) then
             hGetFileSizeAsLong() = filelen( cFileName )
          else
             hGetFileSizeAsLong() = -1
          endif
      catch
          qaerrorlog( CFN & "File was not handled correctly: " & cFileName )
          hGetFileSizeAsLong() = 0
      endcatch

   else

      '///+<li>Return a size of 0 if the file was not found</li>
      hGetFileSizeAsLong() = 0
      warnlog( CFN & "File does not exist: " & cFileName )
                
   endif
   
   '///</ul>

end function

'*******************************************************************************

function hGetWorkFile( cFileName as string ) as string

    '///<h3>Returns a fully qualified filename to a workfile</h3>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Filename without path (string)</li>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Filename including path to user work directory (string)</li>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>
    
    '///+<li>Concatenate workpath and filename, convertpath</li>
    hGetWorkFile() = convertpath( hGetWorkPath() & cFileName )
    '///</ul>

end function


