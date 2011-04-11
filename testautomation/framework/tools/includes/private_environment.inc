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
'*  short description : Tools to ease working with private environment
'*
'\******************************************************************************

function hGetPrivateHttpServer( _proxy as string, _port as string ) as boolean


    '///<h3>Retrieve the name and port for a private http server</h3><br>

    '///<u>Function parameters:</u><br>
    '///<ol>

    '///+<li>Fully qualified server name incl. domain (string)</li>
    '///<ul>
    '///+<li>The string comes without a protocol part (no http://)</li>
    '///</ul>

    '///+<li>Server port (string)</li>
    '///<ul>
    '///+<li>Please remember limits for integer data type</li>
    '///</ul>

    '///</ol>


    '///<u>Returns:</u><br>

    '///<ol>
    '///+<li>Error condition (boolean)</li>
    '///<ul>
    '///+<li>&quot;TRUE&quot;: Data was retrieved successfully</li>
    '///+<li>&quot;FALSE&quot;: Any error </li>
    '///</ul>
    '///</ol>
    
    use "global\tools\includes\optional\t_key_tools.inc"

    const CFN = "hGetPrivateHttpServer::"
    dim cFile as string
    dim cDataSet( 100 ) as string
    dim iErr as integer

    '///<u>Description:</u>
    '///<ul>
    '///+<li>Get the source file</li>
    cFile = hGetPrivateInputFile( false )
    printlog( CFN & "Using input file: " & cFile )
    
    '///+<li>Retrieve the information from the datafile</li>
    iErr = hGetDataFileSection( cFile, cDataSet(), "http_proxy", "", "" )
    if ( iErr = 0 ) then
        hGetPrivateHttpServer() = false
        exit function
    endif
    
    '///+<li>Extract the proxy name</li>
    _proxy = hGetValueForKeyAsString( cDataSet(), "name" )
    
    '///+<li>Append the domain</li>
    _proxy = _proxy & hGetValueForKeyAsString( cDataSet(), "domain" )
    
    '///+<li>Get the port</li>
    _port = hGetValueForKeyAsString( cDataSet() , "port" )
    
    '///+<li>Print a log</li>
    printlog( CFN & _proxy & ":" & _port )
    '///</ul>

    hGetPrivateHttpServer() = true

end function

'*******************************************************************************

function hGetPrivateFtpServer( _proxy as string, _port as string ) as boolean


    '///<h3>Retrieve the name and port for a private ftp server</h3><br>

    '///<u>Function parameters:</u><br>
    '///<ol>

    '///+<li>Fully qualified server name incl. domain (string)</li>
    '///<ul>
    '///+<li>The string comes without a protocol part (no ftp://)</li>
    '///</ul>

    '///+<li>Server port (string)</li>
    '///<ul>
    '///+<li>Please remember limits for integer data type</li>
    '///</ul>

    '///</ol>


    '///<u>Returns:</u><br>

    '///<ol>
    '///+<li>Error condition (boolean)</li>
    '///<ul>
    '///+<li>&quot;TRUE&quot;: Data was retrieved successfully</li>
    '///+<li>&quot;FALSE&quot;: Any error </li>
    '///</ul>
    '///</ol>
    
    use "global\tools\includes\optional\t_key_tools.inc"

    const CFN = "hGetPrivateFtpServer::"
    dim cFile as string
    dim cDataSet( 100 ) as string
    dim iErr as integer

    '///<u>Description:</u>
    '///<ul>
    '///+<li>Get the source file</li>
    cFile = hGetPrivateInputFile( false )
    printlog( CFN & "Using input file: " & cFile )
    
    '///+<li>Retrieve the information from the datafile</li>
    iErr = hGetDataFileSection( cFile, cDataSet(), "ftp_proxy", "", "" )
    if ( iErr = 0 ) then
        hGetPrivateFtpServer() = false
        exit function
    endif
    
    '///+<li>Extract the proxy name</li>
    _proxy = hGetValueForKeyAsString( cDataSet(), "name" )
    
    '///+<li>Append the domain</li>
    _proxy = _proxy & hGetValueForKeyAsString( cDataSet(), "domain" )
    
    '///+<li>Get the port</li>
    _port = hGetValueForKeyAsString( cDataSet() , "port" )
    
    '///+<li>Print a log</li>
    printlog( CFN & _proxy & ":" & _port )
    '///</ul>

    hGetPrivateFtpServer() = true

end function

'*******************************************************************************

function hGetPrivateSocksServer( _proxy as string, _port as string ) as boolean


    '///<h3>Retrieve the name and port for a private socks server</h3><br>

    '///<u>Function parameters:</u><br>
    '///<ol>

    '///+<li>Fully qualified server name incl. domain (string)</li>

    '///+<li>Server port (string)</li>
    '///<ul>
    '///+<li>Please remember limits for integer data type</li>
    '///</ul>

    '///</ol>


    '///<u>Returns:</u><br>

    '///<ol>
    '///+<li>Error condition (boolean)</li>
    '///<ul>
    '///+<li>&quot;TRUE&quot;: Data was retrieved successfully</li>
    '///+<li>&quot;FALSE&quot;: Any error </li>
    '///</ul>
    '///</ol>
    
    use "global\tools\includes\optional\t_key_tools.inc"

    const CFN = "hGetPrivateSocksServer::"
    dim brc as boolean 'a multi purpose boolean returnvalue
    dim cFile as string
    dim cDataSet( 100 ) as string
    dim iErr as integer

    '///<u>Description:</u>
    '///<ul>
    '///+<li>Get the source file</li>
    cFile = hGetPrivateInputFile( false )
    
    '///+<li>Retrieve the information from the datafile</li>
    iErr = hGetDataFileSection( cFile, cDataSet(), "socks", "", "" )
    if ( iErr = 0 ) then
        hGetPrivateSocksServer() = false
        exit function
    endif
    
    '///+<li>Extract the proxy name</li>
    _proxy = hGetValueForKeyAsString( cDataSet(), "name" )
    
    '///+<li>Append the domain</li>
    _proxy = _proxy & hGetValueForKeyAsString( cDataSet(), "domain" )
    
    '///+<li>Get the port</li>
    _port = hGetValueForKeyAsString( cDataSet() , "port" )
    
    '///+<li>Print a log</li>
    printlog( CFN & _proxy & ":" & _port )
    '///</ul>

    hGetPrivateSocksServer() = true

end function

'*******************************************************************************

function hGetPrivateInputPath( bVerbose as boolean ) as string


    '///<h3>Switch the private input path from ../inc/ to ../input/</h3>
    
    '/// This is part of the effort to allow usage of private data such as
    '///+ servernames, paths, login names etc. within a Local Area Network (LAN).  
    '/// If you have to maintain data of any sort that may not get out to the
    '///+ public you will want to put your data into this directory.<br><br>
    
    '///<u>Input Value:</u><br>
    '///<ol>
    '///+<li>Toggle verbosity (boolean)</li>
    '///<ul>
    '///+<li>&quot;TRUE&quot;: Print path</li>
    '///+<li>&quot;FALSE&quot;: Do not print path</li>
    '///</ul>
    '///</ol>
    
    '///<u>Return Value:</u><br>
    '///<ol>
    '///+<li>Path to the private input directory (string)</li>
    '///<ul>
    '///+<li>Function is not supposed to fail</li>
    '///+<li>Fully qualified path to the input directory including last pathseparator</li>
    '///</ul>
    '///</ol>

    const CFN = "hGetPrivateInputPath::"
    dim cPrivateInputPath as string

    '///<u>Description:</u>
    '///<ul>
    '///+<li>Get the ../inc-path from global variable (assuming path is valid)</li>
    cPrivateInputPath = gTesttoolPath & gPrivateEnvironmentLocation
    
    '///+<li>Append input/ (platform specific)</li>
    cPrivateInputPath = cPrivateInputPath & "input/"
    cPrivateInputPath = convertpath( cPrivateInputPath )
    
    '///+<li>Print the resulting path to the log</li>
    if ( bVerbose ) then 
        printlog( CFN & "Private input directory is: " & cPrivateInputPath )
    endif

    hGetPrivateInputPath() = cPrivateInputPath
    '///</ul>

end function

'*******************************************************************************

function hGetPrivateInputFile( bVerbose as boolean ) as string


    '///<h3>Retrieve the fully qualified path to the datafile</h3><br>

    '///<u>Input:</u>
    '///<ol>
    '///+<li>Toggle verbosity (boolean)</li>
    '///<ul>
    '///+<li>&quot;TRUE&quot;: Print path</li>
    '///+<li>&quot;FALSE&quot;: Do not print path</li>
    '///</ul>    
    '///</ol>


    '///<u>Returns:</u>

    '///<ol>
    '///+<li>Fully qualified path to the datafile (string)</li>
    '///</ol>

    const CFN = "hGetPrivateInputFile::"
    dim brc as boolean 'a multi purpose boolean returnvalue
    dim cFile as string

    '///<u>Description:</u>
    '///<ul>
    '///+<li>Append pathseparator and filename to input path</li>
    cFile = hGetPrivateInputPath( false ) & "framework_network.txt"
    
    '///+<li>Print path and filename to the log</li>
    if ( bVerbose ) then
        printlog( CFN & "Private datafile is: " & cFile )
    endif
    
    '///</ul>
    
    hGetPrivateInputFile() = cFile

end function



