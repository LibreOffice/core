'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: t_proxy_info.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-13 10:27:08 $
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
'/******************************************************************************
'*
'*  owner : joerg.skottke@sun.com
'*
'*  short description : Tools to retrieve auxilary environment information
'*
'*******************************************************************************
'**
' #1 hGetProxyInfo ' Retrieve the names and ports of the proxies
' #0 hPrintProxyInstructions ' print instructions to the log
'**
'\******************************************************************************

function hGetProxyInfo( sSection as string, sItem as string ) as string


    '///<h3>Retrieve the names and ports of the proxies</h3>
    '///<i>This function retrieves the names and ports of the proxies used
    '///+ in your local network. The data is taken from a textfile that is
    '///+ not visible on OpenOffice.org (Sun security policy)<br>
    '///+ If the file is not found a message is printed to the log that
    '///+ gives instructions on how to create and format such a file and
    '///+ what other steps must be taken to make this work for your local
    '///+ network (don't panic, it's simple!).</i><br><br>

    '///<u>Input values:</u><br>
    '///<ol>

    '///+<li>Section (string). Valid options:</li>
    '///<ul>
    '///+<li>&quot;http_proxy&quot;</li>
    '///+<li>&quot;ftp_proxy&quot;</li>
    '///+<li>&quot;socks_proxy&quot;</li>
    '///+<li>&quot;no_proxy_for&quot;</li>
    '///</ul>
    
    '///+<li>Item (string). Valid options:</li>
    '///<ul>
    '///+<li>&quot;Name&quot;</li>
    '///+<li>&quot;Port&quot; (not for &quot;no_proxy_for&quot)</li>
    '///</ul>    

    '///</ol>


    '///<u>Return Value:</u><br>
    '///<ol>
    '///+<li>Name or port of an item (string)</li>
    '///<ul>
    '///+<li>Empty String on error</li>
    '///</ul>
    '///</ol>

    const CFN = "hGetProxyInfo::"

    dim irc as integer ' some integer returnvalue
    dim crc as string  ' some string returnvalue
        
    ' This is the workfile. Make sure it exists and contains valid data
    dim cFile as string
        cFile = gTesttoolPath & "sun_global\input\proxies.txt"
        ' cFile = gTesttoolPath & "global\input\proxies.txt"
        cFile = convertpath ( cFile )
       
    ' this is a temporary list that holds the workfile 
    dim acList( 50 ) as string

    '///<u>Description:</u>
    '///<ul>
    '///+<li>Open the file, read the section, abort on error</li>
    irc = hGetDataFileSection( cFile, acList(), sSection , "" , "" )
    if ( irc = 0 ) then
        qaerrorlog( CFN & "File or section not found" )
        hGetProxyInfo() = ""
        hPrintProxyInstructions()
        exit function
    endif
    
    '///+<li>Isolate the key</li>
    crc = hGetValueForKeyAsString( acList(), sItem )
    if ( instr( crc , "Error:" ) > 0 ) then
        qaerrorlog( CFN & "The requested item could not be found" )
        hGetProxyInfo() = ""
        hPrintProxyInstructions()
        exit function
    endif
    
    '///+<li>Return the requested item</li>
    
    '///</ul>

    hGetProxyInfo() = crc 

end function

'*******************************************************************************

function hPrintProxyInstructions()

    printlog( "" )
    printlog( "How to configure proxy settings for your local network" )
    printlog( "" )
    printlog( "1. Edit the sample configuration file" )
    printlog( "   Location: global/input/proxies.txt" )
    printlog( "   Replace servernames and ports with valid entries" )
    printlog( "" )
    printlog( "2. Edit the function hGetProxyInfo" )
    printlog( "   Make the first line with cFile = ... a comment"
    printlog( "   Make the second line with cFile = ... active"
    printlog( "   Save the file" )
    printlog( "" )
   
end function
