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
'* short description : Exit the [Star|Open][Office|Suite][.org] and delete user directory
'*
'\*************************************************************************************

sub main
    dim sString as string

    sString = "qatesttool/global/tools/resetoffice.bas:: "
    
        'Close OOo
        try
            ' To prevent restarting of OOo, the try/catch is around this and
            ' to prevent messages about communication errors
            printlog ResetApplication
            FileExit "SynchronMode", TRUE
        catch
            warnlog sString + "Failed to close OOo."
        endcatch
        sleep 10 'To wait until OOo is realy away

    'Remove user directory
        try
            if (right(gOfficePath,1)=gPathSigne) then
                'Dir doesn't work, is a path singe is at the end
                gOfficePath = left(gOfficePath,len(gOfficePath)-1)
            endif
            printlog "Going to delete directory: '" + gOfficePath + "'"
            if (dir(gOfficePath) = "") then
                qaErrorlog "Directory is already deleted."
            else
                rmDir (gOfficePath)
                if (dir(gOfficePath) <> "") then
                    warnlog "Directory wasn't deleted."
                endif
            endif
        catch
            warnlog sString + "Failed to delete user directory."
        endcatch
end sub

sub LoadIncludeFiles
   use "global\system\includes\master.inc"
   use "global\system\includes\gvariabl.inc"
   Call GetUseFiles()
end sub

