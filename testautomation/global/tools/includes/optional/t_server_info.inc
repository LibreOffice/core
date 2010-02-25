'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
'/******************************************************************************
'*
'*  owner : joerg.skottke@sun.com
'*
'*  short description : Tools to retrieve information about available servers
'*
'*******************************************************************************
'**
' #1 hGetServerInfo ' Retrieve names and services of some servers
' #0 hPrintServerInstructions ' print instructions to the log
'**
'\******************************************************************************

function hGetServerInfo( sType as string, sItem as string ) as string


    '///<h3>Retrieve names and services of some servers</h3>
    '///<i>This function retrieves some information about available
    '///+ servers within the LAN. These services can be anything like
    '///+ http-server, ftp etc.<br>
    '///+ Please make sure you read the comments in the configuration
    '///+ file to fully understand what this thing does.</i><br><br>

    '///<u>Input values:</u><br>
    '///<ol>

    '///+<li>Type (string). Valid options:</li>
    '///<ul>
    '///+<li>&quot;http_internal&quot;</li>
    '///+<li>&quot;http_external&quot;</li>
    '///+<li>&quot;ftp_internal&quot;</li>
    '///+<li>&quot;ftp_external&quot;</li>
    '///</ul>
    
    '///+<li>Item (string). Valid options:</li>
    '///<ul>
    '///+<li>&quot;Name&quot; to get a name for the server</li>
    '///+<li>&quot;Port&quot; to get the server's port</li>
    '///+<li>&quot;Protocol&quot; to get the supported protocol</li>
    '///+<li>&quot;URL&quot; to get the url (e.g. www.heise.de)</li>
    '///+<li>&quot;UseProxy&quot; to find out whether proxies are needed or not</li>
    '///+<li>&quot;User&quot; to get a loginname</li>
    '///+<li>&quot;Pass&quot; to get a password</li>
    '///</ul>    

    '///</ol>


    '///<u>Return Value:</u><br>
    '///<ol>
    '///+<li>Name, port or other info of an item (string)</li>
    '///<ul>
    '///+<li>Empty String on error</li>
    '///+<li>Unique name for a server (can be NIS name)</li>
    '///+<li>Port (optional, e.g. 80 for http, 21 for ftp ...)</li>
    '///+<li>Protocol (e.g. &quot;http://&quot; or &quot;ftp://&quot;</li>
    '///+<li>URL like www.mydomain.de</li>
    '///+<li>UseProxy (&quot;yes&quot;/&quot;no&quot;)</li>
    '///+<li>User (some username)</li>
    '///+<li>Password (some password, plain text!)
    '///</ul>
    '///</ol>

    const CFN = "hGetServerInfo::"

    dim irc as integer ' some integer returnvalue
    dim crc as string  ' some string returnvalue
        
    ' This is the workfile. Make sure it exists and contains valid data
    dim cFile as string
        cFile = gTesttoolPath & "sun_global\input\servers.txt"
        ' cFile = gTesttoolPath & "global\input\servers.txt"
        cFile = convertpath ( cFile )
       
    ' this is a temporary list that holds the workfile 
    dim acList( 50 ) as string

    '///<u>Description:</u>
    '///<ul>
    '///+<li>Open the file, read the section, abort on error</li>
    irc = hGetDataFileSection( cFile, acList(), sType , "" , "" )
    if ( irc = 0 ) then
        qaerrorlog( CFN & "File or section not found" )
        hGetServerInfo() = ""
        hPrintServerInstructions()
        exit function
    endif
    
    '///+<li>Isolate the key</li>
    crc = hGetValueForKeyAsString( acList(), sItem )
    if ( instr( crc , "Error:" ) > 0 ) then
        qaerrorlog( CFN & "The requested item could not be found" )
        hGetServerInfo() = ""
        hPrintServerInstructions()
        exit function
    endif
    
    '///+<li>Return the requested item</li>
    
    '///</ul>

    hGetServerInfo() = crc 

end function

'*******************************************************************************

function hPrintServerInstructions()

    printlog( "" )
    printlog( "How to configure server settings for your local network" )
    printlog( "" )
    printlog( "1. Edit the sample configuration file" )
    printlog( "   Location: global/input/servers.txt" )
    printlog( "   Replace servernames, ports etc. with valid entries" )
    printlog( "" )
    printlog( "2. Edit the function hGetServerInfo" )
    printlog( "   Make the first line with cFile = ... a comment"
    printlog( "   Make the second line with cFile = ... active"
    printlog( "   Save the file" )
    printlog( "" )
   
end function
