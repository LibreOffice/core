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
'* owner : wolfram.garten@oracle.com
'*
'* short description : XML Tools
'*
'\***********************************************************************************

sub CreateWorkXMLExportDir ( DirName as string )

    '/*** <i>CreateWorkXMLExportDir</i> creates in the StarOffice/work-
    '/*** +directory a directory. If the same name exists the content (files only)
    '/*** +will be deleted.
    
    Dim OutputPath as string
    Dim Isliste(200) as string
    Dim iCurrentFile as integer
    dim iFileCount as integer

    printlog "Checking work-dir & creating '" & gOfficePath & ConvertPath(DirName) & "'."
    if ( FileExists( gOfficePath & ConvertPath( DirName ) ) ) then

        OutputPath = gOfficePath & ConvertPath( DirName )
        IsListe(0) = 0
        GetFileList ( OutputPath ,  "*", IsListe() )
        iFileCount = KillFileList ( IsListe() )

        for iCurrentFile = 1 to iFileCount
            warnlog IsListe( iCurrentFile ) & " could not be deleted!"
        next iCurrentFile

    else

        app.mkDir( gOfficePath & ConvertPath(DirName))

    endif

end sub

'-------------------------------------------------------------------------------

sub hEnablePrettyPrinting ( optional bDisablePrettyPrinting )

    '/// <i>hEnablePrettyPrinting</i> enables the pretty printing (=better
    '/// +formated view) in XML file export. It takes >10 percent more time
    '/// +to save files!!! OPTIONAL parameter is used to DISABLE it.

    ToolsOptions
    Kontext "ExtrasOptionenDLG"
    if ( ExtrasOptionenDlg.exists( 1 ) ) then

        call hToolsOptions("loadsave","general")

        Kontext "TabSpeichern"
        if IsMissing( bDisablePrettyPrinting ) then

            if ( NoPrettyPrinting.IsChecked() ) then
                NoPrettyPrinting.Uncheck()
                printlog "- XML pretty printing enabled ( xmltool1.inc )"
            endif

        else
            
            if ( Not NoPrettyPrinting.IsChecked() ) then
                NoPrettyPrinting.Check()
                printlog "- XML pretty printing disabled ( xmltool1.inc )"
            endif

        endif
        Kontext "ExtrasOptionenDLG"
        hCloseDialog( ExtrasOptionenDLG, "ok" )
    else
        warnlog( "hEnablePrettyPrinting(): Tools/Options dialog not open" )
    endif

end sub


