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
'/************************************************************************
'*
'* owner : joerg.skottke@sun.com
'*
'* short description : misc functions for simpler listhandling. uses t_lists.inc
'*
'\******************************************************************************

function listdebug( lsList() as string , cComment as string ) as integer

   '///<h3>Write the content of a list plus a comment out to a file.</h3>
   '///<i>It is required that following global variables are defined</i>
   '///<ul>
   '///+<li>LDN = ListDebugName (char) = the basename of the debugfiles</li>
   '///+<li>LDC = ListDebugCounter (int) = a number added to the filename</li>
   '///</ul>

   dim cFile as string

       LDC = LDC + 1
       cFile = LDN & LDC & ".log"

   dim sList( 5 ) as string
   sList( 0 ) = "5"
   sList( 1 ) = ""
   sList( 2 ) = "---------------------------------------------------------------"
   sList( 3 ) = cComment & " - Listsize: " & listcount( lsList() )
   sList( 4 ) = "---------------------------------------------------------------"
   sList( 5 ) = ""

   ListWrite( sList() , cFile , "utf8" )
   ListWriteAppend( lsList() , cFile , "utf8" )

end function

'*******************************************************************************

function initlistdebug( cComment as string ) as integer

    '///<h3>Print leading text to a file and an index of the current debug session</h3>
    '///<i>It is required that following global variables are defined</i>
    '///<ul>
    '///+<li>LDN = ListDebugName (char) = the basename of the debugfiles</li>
    '///+<li>LDC = ListDebugCounter (int) = a number added to the filename</li>
    '///</ul>
    dim cFile as string
        cFile = LDN & ".log"

    dim sList( 5 ) as string
    sList( 0 ) = "5"
    sList( 1 ) = ""
    sList( 2 ) = "==============================================================="
    sList( 3 ) = cComment & " ---- Debug-Offset is at: " & LDC
    sList( 4 ) = "==============================================================="
    sList( 5 ) = ""

    ListWrite( sList() , cFile , "utf8" )

    printlog( "" )
    printlog( " *** Debug is enabled ***" )
    printlog( "" )

end function

'*******************************************************************************

function listmoveitem( source() as string, _
                       target() as string, _
                       itemid as integer ) as integer
                       
    '///<h3>Move one item from one list to another by index</h3>                       
    '///<ul>

    '///+<li>copy the list-item from list A to the end of list B, update listcount</li>
    listappend( target() , source( itemid ) )
    
    '///+<li>Delete the entry from the old list, reindex and update listcount</li>
    listdelete( source() , itemid )
    
    '///+<li>Return then updated listcount of the <i>source</i> list</li>
    listmoveitem() = listcount( source() )
    
    '///</ul>

end function

'*******************************************************************************

function listconvertpath( lsList() as string ) as integer

    '///<h3>Execute <i>convertpath</i> on a list containing filepaths</h3>
    '///<ul>

    dim iCurrentPath as integer

    '///+<li>Convert all listitems with <i>convertpath</i></li>
    for iCurrentPath = 1 to listcount( lsList() )
        lsList( iCurrentPath ) = convertpath( lsList( iCurrentPath ) )
    next iCurrentPath

    '///+<li>Return the number of processed paths (listcount)</li>
    listconvertpath() = listcount( lsList() )
   
    '///</ul>

end function

'*******************************************************************************

function listInsertSection( lsList() as string, _
                            cSection as string ) as integer
                            
    '///<h3>Appends a section (as ordinary list element) to a list</h3>
    '///<ul>
                            
    dim iPos as integer
    dim sSectionString as string
    
    '///+<li>Get the current number of entries from the list</li>
    iPos = listcount( lsList() )
    
    '///+<li>Insert a blank list-entry if we are not at the beginning of the list</li>
    if ( iPos > 2 ) then
        listappend( lsList() , "" )
    end if                            
    
    '///+<li>Build the section string of type [section-name]</li>
    sSectionString = "[" & cSection & "]"
    
    '///+<li>Append the new section to the list</li>
    listappend( lsList() , sSectionString )
    
    '///+<li>Return the new number of entries in the list (listcount)</li>
    listInsertSection() = listcount( lsList() )
    
    '///</ul>

end function   
