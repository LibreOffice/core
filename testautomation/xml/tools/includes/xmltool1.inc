'encoding UTF-8  Do not remove or change this line!
'*************************************************************************
'
'  Licensed to the Apache Software Foundation (ASF) under one
'  or more contributor license agreements.  See the NOTICE file
'  distributed with this work for additional information
'  regarding copyright ownership.  The ASF licenses this file
'  to you under the Apache License, Version 2.0 (the
'  "License"); you may not use this file except in compliance
'  with the License.  You may obtain a copy of the License at
'  
'    http://www.apache.org/licenses/LICENSE-2.0
'  
'  Unless required by applicable law or agreed to in writing,
'  software distributed under the License is distributed on an
'  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
'  KIND, either express or implied.  See the License for the
'  specific language governing permissions and limitations
'  under the License.
'
'*************************************************************************
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


