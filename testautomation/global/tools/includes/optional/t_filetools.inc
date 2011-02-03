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
'*  owner : gregor.hartmann@oracle.com
'*
'*  short description : Functions that work with files like deleting, loading...
'*
'\******************************************************************************

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

    if ( GVERBOSE ) then printlog( CFN & "Enter" )

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
                    printlog( CFN & "Missed - this dialog has no Cancel button (create new document)" )
                    active.ok()
                endcatch

                if ( not bSkip ) then                
                    if ( Active.getRT() = 304 ) then
                        if ( Active.getButtonCount() = 1 ) then
                            active.ok()
                            printlog( CFN & "MSG (" & iActives & "): closed with OK" )
                            bSkip = true
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
            kontext "Active"
            if ( Active.exists( 1 ) ) then
                if ( Active.getButtonCount() = 2 ) then
                    printlog( CFN & "<Update links dialog> or <File corruption warning>" )
                    active.yes()
                    hFileWait( FALSE )
                    kontext "Active"
                    if ( Active.exists( 2 ) ) then
                        qaerrorlog( "#105670# - File corruption warning, the template is broken" )
                        qaerrorlog( CFN & "Trying to recover." )
                        Active.ok()
                        hHandleActivesOnLoad() = FALSE
                        exit function
                    endif
                endif
            endif

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
        if ( GVERBOSE ) then printlog( CFN & "Exit" )
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
        
    if ( GVERBOSE ) then printlog( CFN & "Enter" )
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
            Cancel.Click()
        endif

        '///+<li>Test for StarOfficeCalendar2-dialog (This should be idential to StarofficeCalendar1)</li>
        Kontext "StarOfficeCalendar2"
        if ( StarOfficeCalendar2.Exists() ) then
            printlog( CFN & "Closing StarOfficeCalendar2-dialog" )
            incident = 5
            Cancel.Click()
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
    if ( GVERBOSE ) then printlog( CFN & "Exit" )
    
    '///</ul>

end function         

'*******************************************************************************


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



