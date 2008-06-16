'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: filedlg_ftp_proxy.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 12:18:14 $
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
'*  short description : Load/Save files using direct connection or proxy
'*
'\******************************************************************************

testcase tFiledlgFtpProxy

    '///<h3>Open a ftp connection to a server directly and via proxy</h3>
    '///<p>This testcase uses the private environment feature, you need to be 
    '///+ behind a firewall for this to work.</p>
    '///<ul>
    
    const URL = "ftp://ftp.mozilla.org"
    
    dim cProxyName as string
    dim cProxyPort as string
    
    dim brc as boolean
    
    '///+<li>Make sure we have exactly one document open and that this is &quot;The first doc!&quot;</li>
    do while( getDocumentCount > 0 ) 
        hCloseDocument()
    loop
    
    '///+<li>Retrieve proxy configuration data</li>
    brc = hGetPrivateFtpServer( cProxyName, cProxyPort )
    if ( not brc ) then
        warnlog( "Could not retrieve valid proxy configuration, aborting test" )
        goto endsub
    endif
    
    '///+<li>Configure a valid proxy (uses private environment data)</li>
    hSetProxies( "", "", cProxyName, cProxyPort, "" )
    
    '///+<li>Load the page</li>
    printlog( "FileOpen / Set URL / Load the file" )
    FileOpen
    kontext "OeffnenDlg"
    if ( OeffnenDlg.exists( 2 ) ) then
        Dateiname.setText( URL )
        Oeffnen.click()
        sleep( 10 )
    else
        warnlog( "File open dialog not open" )
        goto endsub
    endif
    
    '///+<li>Verify that the document is loaded</li>
    brc = hIdentifyWriterDoc( "Current directory is /" , true )
    if ( not brc ) then 
        warnlog( "This is either the incorrect file or we failed to load the document" )
    endif
    
    '///+<li>Close the loaded document</li>
    hDestroyDocument()
    
    '///+<li>Reset the proxies to "none" (manual, with no proxy settings)</li>
    hSetProxies( "", "", "", "", "" )
    
    '///+<li>Try to load the file with no proxies -> should fail</li>
    '///<ul>
    '///+<li>FileOpen</li>
    FileOpen
    
    '///+<li>Enter filename, click &quot;Open&quot;</li>
    kontext "OeffnenDlg"
    if ( OeffnenDlg.exists( 2 ) ) then
        DateiName.setText( URL )
        Oeffnen.click()
    
        '///+<li>Close the &quot;File not found&quot; warning with &quot;OK&quot;</li>
        Kontext "active"
        if ( active.exists( 5 ) ) then
            active.ok()
            
            '///+<li>Close the FileOpen dialog</li>
            kontext "OeffnenDlg"
            OeffnenDlg.cancel()
        else
            kontext "OeffnenDlg" 
            if ( OeffnenDlg.exists() ) then
                qaerrorlog( "Document not loaded / Still on File Open dialog" )
                OeffnenDlg.cancel()
            else
                warnlog( "File not found warning is missing" )
            endif
        endif
    endif
    
    '///</ul>
    '///</ul>


endcase

