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
'*  short description : Tools to ease working with web functionality
'*
'\******************************************************************************

function hSetProxies( HTTP_Server as string, HTTP_Port as string, _
                      FTP_Server as string, FTP_Port as string, _
                      NoProxyFor as string ) as integer
                      
    use "global\tools\includes\optional\t_control_objects.inc"

    '///<h3>Configure proxy settings via Tools/Options Internet page</h3>
    '///<i>Starting point is a plain document of any type</i><br>
    '///<i>If all fields are empty The proxy will be set to &quot;None&quot;</i><br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>HTTP Server name (string)</li>
    '///<ul>
    '///+<li>As name, e.g. myProxy.com (FQDN recommended)</li>
    '///+<li>As IP, e.g. 192.168.1.254</li>
    '///+<li>Non optional parameter which may be an empty string</li>
    '///+<li>Requires HTTP Server port to be set</li>
    '///</ul>
    '///+<li>HTTP Server port</li>
    '///<ul>
    '///+<li>e.g. 8080, 3128</li>
    '///+<li>Non optional parameter which may be an empty string</li>
    '///</ul>
    '///+<li>FTP Server name (string)</li>
    '///<ul>
    '///+<li>As name, e.g. myProxy.com (FQDN recommended)</li>
    '///+<li>As IP, e.g. 192.168.1.254</li>
    '///+<li>Non optional parameter which may be an empty string</li>
    '///+<li>Requires FTP Server port to be set</li>
    '///</ul>
    '///+<li>FTP Server port</li>
    '///<ul>
    '///+<li>e.g. 8080, 3128</li>
    '///+<li>Non optional parameter which may be an empty string</li>
    '///</ul>  
    '///+<li>Exclude domains (string)</li>
    '///<ul>
    '///+<li>e.g. .Sun.COM;.Germany.Sun.Com;MyServer.Sun.Com</li>
    '///+<li>Non optional parameter which may be an empty string</li>
    '///</ul> 
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Errorcondition</li>
    '///<ul>
    '///+<li>0 = Successfully set/unset proxies</li>
    '///+<li>1 - 5 = Number of failed actions</li>
    '///</ul>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>
    
    const CFN = "hSetProxies::"
    dim iControlStatus as integer
    dim irc as integer
        irc = 0
    
    '///+<li>Open Tools/Options</li>
    ToolsOptions

    '///+<li>Go to the Proxy Settings (Internet/Proxy)</li>
    hToolsOptions( "Internet", "Proxy" )
    
    '///+<li>Select Manual Proxy (3. Item in the list)</li>
    ProxyServer.select( 3 )
         
    '///+<li>Set the new values</li>
    '///<ol>
    
    '///+<li>HTTP-Proxy</li>
    iControlStatus = hSetControlValue( HTTPProxy , HTTP_Server )
    if ( iControlStatus <> 0 ) then
        qaerrorlog( CFN & "Failed to set HTTP-Proxy" )
        irc = irc + 1
    endif
    
    '///+<li>HTTP-Port</li>
    iControlStatus = hSetControlValue( HTTPPort  , HTTP_Port  )
    if ( iControlStatus <> 0 ) then
        qaerrorlog( CFN & "Failed to set HTTP-Port" )
        irc = irc + 1
    endif
    
    '///+<li>FTP-Proxy</li>
    iControlStatus = hSetControlValue( FTPProxy  , FTP_Server  )
    if ( iControlStatus <> 0 ) then
        qaerrorlog( CFN & "Failed to set FTP-Proxy" )
        irc = irc + 1
    endif
    
    '///+<li>FTP-Port</li>
    iControlStatus = hSetControlValue( FTPPort   , FTP_Port   )
    if ( iControlStatus <> 0 ) then
        qaerrorlog( CFN & "Failed to set FTP-Port" )
        irc = irc + 1
    endif
    
    '///+<li>Exclusion list</li>
    iControlStatus = hSetControlValue( NoProxy   , NoProxyFor )
    if ( iControlStatus <> 0 ) then
        qaerrorlog( CFN & "Failed to set exclusion list" )
        irc = irc + 1
    endif
    '///</ol>
    
    if ( irc = 0 ) then
        printlog( CFN & "Successfully updated proxy configuration" )
    endif
    
    '///+<li>Close Tools/Options</li>
    Kontext "ExtrasOptionenDlg"
    ExtrasOptionenDlg.OK()

    hSetProxies() = irc
    '///</ul>
    

end function


