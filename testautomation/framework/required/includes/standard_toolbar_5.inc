'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: standard_toolbar_5.inc,v $
'*
'* $Revision: 1.1 $
'*
'* last change: $Author: jsi $ $Date: 2008-06-16 12:19:03 $
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
'* owner : joerg.skottke@sun.com
'*
'* short description : global update test (Standardbar)
'*
'\***************************************************************************


testcase tStandardBar_5
    
    '///<h1>Update test: Configure the Standard Bar via Tools/Customize</h1>
    '///<ul>
    
    warnlog( "#i106370# - Test disabled, cannot switch tabpage on Tools/Customize dialog" )
    goto endsub
    
    '///+<li>open a new document</li>
    hCreateDocument()
    
    '///+<li>open context menu on Standardbar and select the 2'th item (Customize)</li>
    printlog "Customize ..."

    kontext "standardbar"
    Standardbar.OpenContextmenu()
    waitslot
    hMenuselectNr( 2 )
    waitslot
    
    '///+<li>Select all tabpages ( Menu, Keyboard, Toolbars, Events )</li>
    '///<ol>
    '///+<li>Menu</li>
    hToolsCustomizeSelectTab( "menu" )
    call dialogtest( TabCustomizeMenu )

    '///+<li>Keyboard</li>
    hToolsCustomizeSelectTab( "keyboard" )
    call dialogtest( Tabtastatur )
    
    '///+<li>Toolbars</li>
    hToolsCustomizeSelectTab( "toolbars" )
    call dialogtest( TabCustomizeToolbars )
    
    '///+<li>Events</li>
    hToolsCustomizeSelectTab( "events" )
    call dialogtest( TabCustomizeEvents )
    
    '///+<li>Click on 'OK' at Event-Tabpage</li>
    kontext "tabcustomizeevents"
    TabCustomizeEvents.OK()
    '///</ol>
    
    '///+<li>Open context menu on Standardbar and select the 2nd item (Customize Toolbar)</li>
    kontext "standardbar"
    Standardbar.OpenContextmenu()
    waitslot
    hMenuselectNr (2)
    waitslot
    
    '///+<li>Close the Customize-Toolbars-Dialog with OK</li>
    kontext "tabcustomizetoolbars"
    TabCustomizeToolbars.OK()
    
    '///+<li>Reset the StandardBar to defaults</li>
    printlog "Reset"
    hResetStandardBar()
    
    '///+<li>close the document</li>
    hDestroyDocument()
    '///</ul>
    
endcase


