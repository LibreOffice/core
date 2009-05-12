'encoding UTF-8  Do not remove or change this line!
'**************************************************************************
'* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
'* 
'* Copyright 2008 by Sun Microsystems, Inc.
'*
'* OpenOffice.org - a multi-platform office productivity suite
'*
'* $RCSfile: t_user_info.inc,v $
'*
'* $Revision: 1.1.2.1 $
'*
'* last change: $Author: jsk $ $Date: 2008/08/27 10:09:36 $
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
'*  short description : Extension Update Test
'*
'\******************************************************************************

function hCheckForAdministratorPermissions() as boolean

    ' this function returns TRUE if the user can create files in the office
    ' program directory

    dim iFile as integer
    dim cProbeFile as string
    
    cProbeFile = convertpath( gNetzOfficePath & "program/tt_probe_file" )
    try
        iFile = FreeFile
        open cProbeFile for output as iFile : close iFile
        kill cProbeFile
        hCheckForAdministratorPermissions() = true
        printlog( "Current user has administrator rights" )
    catch
        hCheckForAdministratorPermissions() = false
        printlog( "Current user does not have administrator rights" )
    endcatch

end function
