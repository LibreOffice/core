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
'* owner : thorsten.bosbach@oracle.com
'*
'* short description : routines to handle ini-files ( read/write items )
'*
'*****************************************************************
'*
' #1 GetIniValue          ' get a value of an entry
' #1 GetIniValue2         '  subroutine for GetIniValue
' #1 SetIniValue          ' set a value of an entry
' #1 SetIniValue2         '   subroutine for SetIniValue
' #1 ChangeExt            ' change the extension of an file
' #1 AnhaengenAnDatei     ' add a string into a file
'*
'\****************************************************************

function GetIniValue ( Datei$, Gruppe$, Variable$ ) as String
    '/// wrapper for GetIniValue2 ///'
    '///+ reads a value from an ini-file ///'
    '///+ INPUT : name of ini-file; name of group (the one in braces []); the item (left of '=') ///'
    '///+ OUTPUT: value (the right of the '=') ///'
    if Dir(Datei$) = "" then
        Warnlog "Error in GetIniValue(...):" + Datei$ + " not found"
        exit function
    end if

    GetIniValue = GetIniValue2( Datei$, Gruppe$, Variable$ ) ' Arbeiten
end function

function SetIniValue( Datei$, Gruppe$, Variable$, Value$ ) as String
    '/// wrapper for SetIniValue2 ///'
    '///+ writes a value to an ini-file ///'
    '///+ INPUT : name of ini-file; name of group (the one in braces []); the item (left of '='); value (the right of the '=') ///'
    '///+ OUTPUT: - ///'
    Dim FileNum as Integer

    if Dir(Datei$) = "" then
        WarnLog "Error in SetIniValue(...):" + Datei$ + " not found. File will be created now!"
        FileNum = FreeFile
        Open Datei$ For Output As #FileNum  ' make empty file
        Print #FileNum, ""
        Close #FileNum
    end if

    SetIniValue = SetIniValue2( Datei$, Gruppe$, Variable$, Value$ )
end function


function GetIniValue2( Datei$, Gruppe$, Variable$ ) as String
    '/// see the wrapper for it : GetIniValue ///'
    Dim FileNum% : Dim GruppeOK% :  Dim Pos% :  Dim IniZeile$ :  Dim IniZeile2$

    FileNum% = FreeFile

    GruppeOK%=FALSE

    GetIniValue2 = ""

    Open Datei$ For Input As #FileNum%
    do until EOF(#FileNum%) = True
        Line input #FileNum%, IniZeile$

        IniZeile$ = TRIM(IniZeile$)
        iniZeile2$ = UCASE( IniZeile$ ) ' compare case insensitive
        if GruppeOK% = FALSE then     ' still no group
            if IniZeile2$= "[" + UCASE( Gruppe$ ) + "]" then                     'Is it the wanted group?
                GruppeOK% = TRUE
            end if
        else
            If Left(IniZeile2$, 1) = "[" then                                 'sadly new group - goodby
                Exit do
            else
                Pos% = Instr( IniZeile2$, "=" )                           'is the item valid?
                if Pos%>0 then                                                    ' '=' not found
                    if Left( IniZeile2$ , Pos%-1 ) = UCASE( Variable$ ) then       'compare leftvalue
                        GetIniValue2 = Trim(Mid$( IniZeile$ , Pos%+ 1  ))           'return part right of '=' : with initial case
                        exit do
                    end if
                end if
            end if
        end if

    loop

    Close #FileNum%
    wait 1000
end function


sub SetIniValue2( Datei$, Gruppe$, Variable$, Value$ ) as String
    '/// see the wrapper for it : SetIniValue ///'
    Dim DateiBak$ : Dim D$ : Dim IniZeile$ : Dim IniZeile2$
    Dim FileBak% : Dim GruppeOK% : Dim Gefunden% : Dim FileNum% : Dim Pos%

    ' rename
    DateiBak$ = ChangeExt( Datei$, "BAK" )

    GruppeOK% = FALSE
    Gefunden% = FALSE

    if Dir(DateiBak$)<>"" then
        kill DateiBak$
    end if

    if Dir( Datei$ )<>"" then
        D$ = CurDir
        name Datei$ as DateiBak$
    else
        FileNum% = FreeFile
        Open Datei$ For Output As #FileNum%
        Print #FileNum%, "[" + Trim(Gruppe$) + "]"
        Print #FileNum%, Variable$ + "=" + Trim(Value$)
        Close #FileNum%                                                   ' finished here
        Exit sub
    endif

    FileNum% = FreeFile
    Open Datei$ For Output As #FileNum%

    FileBak% = FreeFile
    Open DateiBak$ For Input As #FileBak%

    do until EOF(#FileBak%) = True

        Line input #FileBak%, IniZeile$

        IniZeile$ =  TRIM(IniZeile$)

        if IniZeile$ <> "" then

            IniZeile2$ = UCASE( IniZeile$ )

            if Left(IniZeile$, 1) = "[" then
                if GruppeOK% = TRUE then                                     'groupchange
                    if Gefunden%=FALSE then
                        Print #FileNum%,  Variable$ + "=" + Trim(Value$)
                        Gefunden% = TRUE
                    end if
                    GruppeOK% = FALSE
                end if
                Print #FileNum%, ""                                            'empty line
                Print #FileNum%, IniZeile$
                if IniZeile2$= "[" + UCASE( Gruppe$ ) + "]" then
                    GruppeOK% = TRUE
                end if
            else
                if GruppeOK% = TRUE then                            ' found group

                    Pos% = Instr( IniZeile$, "=" )
                    if Left( IniZeile2$ , Pos%-1 ) = UCASE( Variable$ ) then
                        IniZeile$ = Left( IniZeile$ , Pos%   ) +Trim( Value$ )' after the '='
                        Gefunden% = TRUE
                    end if
                end if
                Print #FileNum%, IniZeile$
            end if

        end if

    loop

    if Gefunden% = FALSE then
        ' set new group and value
        if GruppeOK%=FALSE then
            Print #FileNum%, ""
            Print #FileNum%, "[" + Trim(Gruppe$) + "]"
        end if
        Print #FileNum%, Variable$ + "=" + Value$
    end if

    Close #FileNum%
    Close #FileBak%

    wait 1000
end sub


sub AnhaengenAnDatei ( Datei as String, Texte as String )
    '/// append a string at the end of the file ///'
    '///+ INPUT : filename; string///'
    '///+ OUTPUT: - ///'
    Dim FileNum%

    FileNum% = FreeFile
    Open Datei for Append  as #FileNum%

    Print #FileNum%, Texte
    Close #FileNum%

end sub

function ChangeExt( Datei$, Ext$ )as String
    '/// change the extension of a file ///'
    '///+ INPUT : filename; extension ///'
    '///+ OUTPUT: - ///'
    Dim i%

    i% = InStr( Right(Datei$, 4 ) , "." )
    if Ext$<>"" then
        if i%=0 then
            ChangeExt = Datei$ +"."+Ext$
        else
            ChangeExt = Left( Datei$, Len(Datei$)-4+i% )   + Ext$
        end if

    elseif i%<>0 then
        ChangeExt = Left( Datei$, Len(Datei$)-5+i% )
    end if
end function

