'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: t_macro_tools.inc,v $
'*
'* $Revision: 1.2 $
'*
'* last change: $Author: rt $ $Date: 2008-07-31 19:26:13 $
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
' **
' ** owner : joerg.skottke@sun.com
' **
' ** short description : Helper functions for Macro tests
' **
'\******************************************************************************

function hInsertMacroFromFile( cMacroId as string, optional cSource as string ) as integer

    '///<h3>Paste a macro (taken from a file) to the basic IDE</h3>
    '///<i>uses: framework/tools/input/macros.txt</i><br>
    '///<i>Starting point: Basic IDE</i><br>
    '///<u>Note</u>: Overwrites any existing macros in the current module<br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Name (ID) of the macro to be inserted (string)</li>
    '///<ul>
    '///+<li>Allowed is any string that corresponds to a section in the source file</li>
    '///</ul>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Number of lines inserted (integer)</li>
    '///<ul>
    '///+<li>0: Error, no lines inserted</li>
    '///+<li>&gt; 0: Number of lines</li>
    '///</ul>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>
    
    const CFN = "hInsertMacroFromFile::"
    
    '///+<li>Find the path to the source file</li>
    dim cFile as string
    if ( IsMissing( cSource ) ) then
        cFile = convertpath( gTesttoolPath & "global/input/macros.txt" )
    else
        cFile = convertpath( cSource )
    endif        
    
    '///+<li>Determine the required array size</li>
    dim iArraySize as integer
        iArraySize = hListFileGetSize( cFile )
        
    dim aInstructionList( iArraySize ) as string
        
    dim iInstructionCount as integer
    dim iCurrentInstruction as integer
    
    dim brc as boolean
    
    '///+<li>retrieve the macro from the file with ID as section</li>
    iInstructionCount = hGetDataFileSection( cFile, _
                                             aInstructionList(), _
                                             cMacroId, "", "" )
                        
    '///+<li>Delete all content from the BASIC IDE edit window</li>
    brc = hDeleteMacro()

    '///+<li>Insert the code into the IDE line by line</li>
    if ( brc ) then

        for iCurrentInstruction = 1 to iInstructionCount
        
            EditWindow.TypeKeys( "<HOME>" )
            EditWindow.TypeKeys( aInstructionList( iCurrentInstruction ) )
            EditWindow.TypeKeys( "<RETURN>" )
            
        next iCurrentInstruction
        printlog( CFN & "Inserted macro: " & cMacroId )
        hInsertMacroFromFile() = iInstructionCount
        
    else
    
        printlog( CFN & "IDE is not empty, will not insert macro" )
        hInsertMacroFromFile() = 0
        
    endif    
    '///</ul>
        
end function

'*******************************************************************************

function hMacroOrganizerRunMacro( cMacroName as string ) as integer

    '///<h3>Execute a macro by name</h3>
    '///<i>Starting point: Any document</i><br>
    '///+<i>The function runs silent</i><br>
    '///<u>Input</u>:
    '///<ol>
    '///+<li>Name of the macro to be run (string)</li>
    '///<ul>
    '///+<li>Any macro that can run by itself (main)</li>
    '///</ul>
    '///</ol>
    '///<u>Returns</u>:
    '///<ol>
    '///+<li>Position of the macro in the treelist (integer)</li>
    '///<ul>
    '///+<li>0 = error</li>
    '///+<li>1-n = position of macro</li>
    '///</ul>
    '///</ol>
    '///<u>Description</u>:
    '///<ul>
    
    dim irc as integer
    const CFN = "hMacroOrganizerRunMacro::"

    '///+<li>Go to Tools/Macros/Organize Macros/OpenOffice.org Basic</li>
    printlog( CFN & "Enter with option: " & cMacroName )
    ToolsMacro_uno    
    
    '///+<li>Find the Macro</li>
    kontext "Makro"
    hExpandAllNodes( MakroAus )
    irc = hSelectNodeByName( MakroAus , cMacroName )
    hExpandNode( MakroAus, irc )
    hSelectNextNode( MakroAus )

    if ( MakroAus.getSelText() = cMacroName ) then
        printlog( CFN & "Matching object found: " & cMacroName )
    else
        irc = 0
    endif
    
    
    '///+<li>Run the macro</li>
    kontext "Makro"
    if ( Ausfuehren.exists( 1 ) ) then
        if ( Ausfuehren.isEnabled( 1 ) ) then
            Ausfuehren.click()
        else
            printlog( CFN & "Could not execute macro, button is disabled" )
            Makro.close()
            irc = 0
        endif
    else
        printlog( CFN & "Control does not exist/context failed" )
        Makro.close()
        irc = 0
    endif
    
    hMacroOrganizerRunMacro() = irc
    printlog( CFN & "Exit with result: " & irc )
    '///</ul>
    
end function
