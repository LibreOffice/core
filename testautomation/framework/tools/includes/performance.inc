'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: performance.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 12:19:06 $
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
'*  short description : Tools to speed up test case performance
'*
'\******************************************************************************

function hWaitForObject( oControl as object, iTime as integer ) as integer


    '///<h3>Wait for a control to get ready</h3>
    '///<i>Wait for a control on a toolbar to get ready for a specified amount 
    '///+ of time. Note that this function runs silent, you have to test the 
    '///+ return value!</i><br><br>

    '///<u>Parameter(s):</u><br>
    '///<ol>

    '///+<li>The control to wait for (Object)</li>
    '///<ul>
    '///+<li>Kontext must be set prior to using this function</li>
    '///+<li>The control must exist</li>
    '///+<li>This function works on most, but not all, controls</li>
    '///</ul>

    '///+<li>Maximum time to wait in milliseconds (Integer)</li>
    '///<ul>
    '///+<li>Must be &gt; 0</li>
    '///+<li>Must be &le; 10000 ms (10 seconds)</li>
    '///</ul>

    '///</ol>


    '///<u>Returns:</u><br>
    '///<ol>
    '///+<li>Time to readiness in milliseconds(Integer)</li>
    '///<ul>
    '///+<li>-1: Control not ready within given timeframe</li>
    '///+<li>-2: Control appears not to exist</li>
    '///+<li>-&ge; 0: Time to readiness </li>
    '///</ul>
    '///</ol>

    const MAX_WAIT = 10000
    const CFN = "hWaitForObject::"
    
    dim _time as integer
    dim _loop as integer
    dim _name as string
    dim _wait as integer
    
    _name = oControl.name()
    

    '///<u>Description:</u>
    '///<ul>
    '///+<li>Verify function parameter validity</li>
    if ( iTime > MAX_WAIT ) then
        _time = MAX_WAIT
    else
        _time = iTime 
    endif
    
    '///+<li>Override timeout</li>
    if ( iTime < 0 ) then
        _wait = abs( iTime )
        printlog( CFN & "<" & _name & " Timeout overridden: " & _wait & " ms" )
    else
        _wait = MAX_WAIT
    endif

    '///+<li>Verify that the control exists and wait</li>
    _loop = 0
    if ( oControl.exists() ) then
        do while( not oControl.isEnabled() )
            _loop = _loop + 1
            wait( 1 )
            if ( _loop = MAX_WAIT ) then
                qaerrorlog( CFN & "Timeout reached. Exiting." )
                hWaitForObject() = -1
                exit function
            endif
        loop
        
        ' We get here when all went well.
        printlog( CFN & "Object was enabled within " & _loop & " milliseconds" )
        hWaitForObject() = _loop
        
    else
        ' This message might appear but can - under certain conditions - be 
        ' incorrect. Some toolbars simply do not react to the .exists() method.
        printlog( CFN & "Non existing control. Exiting." )
        hWaitForObject() = -2
        exit function
    endif
    '///</ul>

end function







