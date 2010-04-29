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
'*  owner : joerg.skottke@sun.com
'*
'*  short description : Test modulenames on the tabbar
'*
'\******************************************************************************

testcase tBasicIdeModuleNames

    dim rc as integer
    dim brc as boolean
    dim i as integer
    dim iCurrentName as integer
    const CMODULE = "TModuleNames"
    dim cTabName as string

    gApplication = "WRITER"
    hCreateDocument()

    ' ------ prerequisites ---------
    
    hInitBasicIde( CMODULE )
    if ( hInsertMacro( 1 ) = 0 ) then
   
        cTabName = hGetTabNameFromOrganizer()

        Randomize
        iCurrentName = Int( 24 * RND ) + 1 ' Range from 1 to 24

        cTabName = hCreateInvalidModuleName( iCurrentName )

        if ( hRenameTab( cTabName ) = 1 ) then
            hHandleInvalidNameWarning( cTabname )
        else
            warnlog( "Warning missing" )
        endif

        cTabname = CMODULE

        if ( hRenameTab( cTabname ) <> 0 ) then
            if ( hHandleInvalidNameWarning( cTabname )  ) then
                warnlog( "Failed to set valid name" )
            endif
        endif


        cTabname = hCreateInvalidModuleName( 0 )

        if ( hRenameTab( cTabName ) <> 0 ) then
            if ( hHandleInvalidNameWarning( cTabname )  ) then
                warnlog( "Failed to set valid name" )
            endif
        endif


        if ( hTestMacro( 1 ) <> 1 ) then
            warnlog( "The open macro-module is not the one that was expected" )
        endif
    else
        warnlog( "Failed to insert macro" )
    endif
    
    hCloseBasicIDE()
    hDestroyDocument()

endcase

'*******************************************************************************

testcase tInvalidModuleNames

    printlog( "Try to give BASIC modules invalid names (which should fail)" )

    const MAX_WAIT = 2
    const OK_BUTTON_ONLY = 1

    dim iCurrentName as Integer
    dim sSeparator as String 
    dim cModuleName as string

    Randomize
    iCurrentName = Int( 24 * RND ) + 1 ' Range from 1 to 24
    cModuleName = hCreateInvalidModuleName( iCurrentName )
    printlog( "Trying module name: " & cModuleName )

    hCreateDocument()
    ToolsMacro_uno

    ' hCreateModuleForDoc should usually end with the BASIC IDE open
    ' but we want it to fail by providing an invalid module name
    if ( hCreateModuleForDoc( cModuleName ) ) then
        warnlog( "Basic-Ide should not open for invalid module names" )
    else
        kontext "Active"
        if ( Active.exists( MAX_WAIT ) ) then
            if( Active.getButtonCount() = OK_BUTTON_ONLY ) then

                printlog( "Messagebox <Invalid name> displayed: " & Active.getText() )
                hCloseDialog( Active, "ok" )

                kontext "NeuesModul"
                if ( NeuesModul.exists( MAX_WAIT ) ) then
                    hCloseDialog( NeuesModul, "cancel" )

                    kontext "Makro"
                    if ( Makro.exists() ) then
                        hCloseDialog( Makro, "close" )
                    else
                        warnlog( "Test should be back on macro dialog" )
                    endif
                else
                    warnlog( "<NeuesModul> dialog should show after failed naming attempt" )
                endif
            else
                warnlog( "Unexpected messagebox: " & Active.getText() )
                hCloseDialog( Active, "close" )
            endif
        else
            warnlog( "<Invalid name> Messagebox missing" )
        endif
    endif

    hFileCloseAll()

endcase

'*******************************************************************************

testcase tValidModuleNames

    printlog( "Try using BASIC reserved names (and others) as module names (which should work)" )

    dim iCurrentName as Integer
    dim sModuleName as String

    Randomize
    iCurrentName = Int( 8 * RND ) + 1 ' Range from 1 to 8

    select case iCurrentName
        case 1  : sModuleName = "option"
        case 2  : sModuleName = "sub"
        case 3  : sModuleName = "function"
        case 4  : sModuleName = "end"
        case 5  : sModuleName = "exit"
        case 6  : sModuleName = "_underscore1"
        case 7  : sModuleName = "underscore_2"
        case 8  : sModuleName = "ThisIsQuiteALongNameForAModuleDontYouThink"
    end select

    hCreateDocument()
    ToolsMacro_uno

    if ( hCreateModuleForDoc( sModuleName ) ) then
        printlog( "The module name was accepted: " & sModuleName )
        hFileCloseAll()
    else
        warnlog( "The module name was not accepted: " & sModuleName )
        goto endsub
    endif
    
endcase

'*******************************************************************************

function hHandleInvalidNameWarning( cTabName as string ) as boolean

    const CFN = "hHandleInvalidNameWarning(): "
    const MAX_WAIT = 1

    kontext "Active"
    if ( Active.exists( MAX_WAIT ) ) then

        printlog( "Invalid name message displayed" )
        printlog( Active.getText() )
        hCloseDialog( Active, "ok" )
        hHandleInvalidNameWarning() = true
        
        kontext "Active"
        if ( Active.exists( MAX_WAIT ) ) then
            warnlog( "Unexpected messagebox displayed: " & Active.getText() )
            hHandleInvalidNameWarning() = false
        endif

        kontext "basicide"
        tabbar.typekeys( "<ESCAPE>" , true )

    else
        warnlog( "Invalid name warning is missing" )
        hHandleInvalidNameWarning() = false
    endif

end function

'*******************************************************************************

function hCreateInvalidModuleName( iName as integer ) as string


    dim sSeparator as string
    
    ' Note: We cannot test for "<" and ">" because these are the characters
    '       that identify keystrokes for the .typeKeys() method.

    select case iName
        case 0  : sSeparator = "_"
        case 1  : sSeparator = "-"
        case 2  : sSeparator = "."
        case 3  : sSeparator = ","
        case 4  : sSeparator = "+"
        case 5  : sSeparator = ":"
        case 6  : sSeparator = "!"
        case 7  : sSeparator = "$"
        case 8  : sSeparator = "("
        case 9  : sSeparator = ")"
        case 10 : sSeparator = "="
        case 11 : sSeparator = "?"
        case 12 : sSeparator = "*"
        case 13 : sSeparator = " "
        case 14 : sSeparator = "&"
        case 15 : sSeparator = "\"
        case 16 : sSeparator = "/"
        case 17 : sSeparator = "§"
        case 18 : sSeparator = """"
        case 19 : sSeparator = "'"
        case 20 : sSeparator = "@"
        case 21 : sSeparator = "["
        case 22 : sSeparator = "]"
        case 23 : sSeparator = "%"
        case 24 : sSeparator = CHR$( 387 )
        case else : sSeparator = ""
    end select

    hCreateInvalidModuleName() = "ttModule" & sSeparator & "X"
    
end function
