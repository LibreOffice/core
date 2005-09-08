#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.11 $
#
#   last change: $Author: rt $ $Date: 2005-09-08 16:26:08 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
#
#*************************************************************************

PRJ=..$/..$/..
PRJNAME=sal
TARGET=uwinapi

USE_DEFFILE=TRUE
LIBTARGET=NO

# --- Settings ----------------------------------

.INCLUDE : settings.mk

# --- Targets ----------------------------------


.IF "$(GUI)"=="WNT"

.IF "$(COMEX)"=="9"
.IF "$(PSDK_HOME)"!=""
# Since the 02/2003 PSDK the "new" linker is needed here.
LINK=$(WRAPCMD) "$(PSDK_HOME)$/Bin$/Win64$/LINK.EXE"
.ENDIF
.ENDIF

SLOFILES=\
        $(SLO)$/CommandLineToArgvW.obj\
        $(SLO)$/CopyFileExA.obj\
        $(SLO)$/CopyFileExW.obj\
        $(SLO)$/DrawStateW.obj\
        $(SLO)$/EnumProcesses.obj\
        $(SLO)$/GetLogicalDriveStringsW.obj\
        $(SLO)$/GetLongPathNameA.obj\
        $(SLO)$/GetLongPathNameW.obj\
        $(SLO)$/GetModuleFileNameExA.obj\
        $(SLO)$/GetModuleFileNameExW.obj\
        $(SLO)$/GetProcessId.obj\
        $(SLO)$/GetUserDefaultUILanguage.obj\
        $(SLO)$/GetUserDomainA.obj\
        $(SLO)$/GetUserDomainW.obj\
        $(SLO)$/GetDiskFreeSpaceExA.obj\
        $(SLO)$/GetDiskFreeSpaceExW.obj\
        $(SLO)$/MoveFileExA.obj\
        $(SLO)$/MoveFileExW.obj\
        $(SLO)$/toolhelp.obj\
        $(SLO)$/DllGetVersion.obj\
        $(SLO)$/DllMain.obj\
        $(SLO)$/ResolveThunk.obj\
        $(SLO)$/snprintf.obj\
        $(SLO)$/snwprintf.obj\
        $(SLO)$/FindFirstVolumeA.obj\
        $(SLO)$/FindFirstVolumeW.obj\
        $(SLO)$/FindNextVolumeA.obj\
        $(SLO)$/FindNextVolumeW.obj\
        $(SLO)$/FindVolumeClose.obj\
        $(SLO)$/FindFirstVolumeMountPointA.obj\
        $(SLO)$/FindFirstVolumeMountPointW.obj\
        $(SLO)$/FindNextVolumeMountPointA.obj\
        $(SLO)$/FindNextVolumeMountPointW.obj\
        $(SLO)$/FindVolumeMountPointClose.obj\
        $(SLO)$/GetVolumeNameForVolumeMountPointA.obj\
        $(SLO)$/GetVolumeNameForVolumeMountPointW.obj\
        $(SLO)$/DeleteVolumeMountPointA.obj\
        $(SLO)$/DeleteVolumeMountPointW.obj\
        $(SLO)$/GetVolumePathNameA.obj\
        $(SLO)$/GetVolumePathNameW.obj\
        $(SLO)$/SetVolumeMountPointA.obj\
        $(SLO)$/SetVolumeMountPointW.obj\
        $(SLO)$/PathAddBackslashW.obj\
        $(SLO)$/PathCompactPathExW.obj\
        $(SLO)$/PathFileExistsW.obj\
        $(SLO)$/PathFindExtensionW.obj\
        $(SLO)$/PathFindFileNameW.obj\
        $(SLO)$/PathIsFileSpecW.obj\
        $(SLO)$/PathIsUNCW.obj\
        $(SLO)$/PathRemoveExtensionW.obj\
        $(SLO)$/PathRemoveFileSpecW.obj\
        $(SLO)$/PathSetDlgItemPathW.obj\
        $(SLO)$/PathStripToRootW.obj

SHL1TARGET=$(TARGET)
SHL1IMPLIB=$(SHL1TARGET)
SHL1DEF=$(MISC)/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=\
    $(SHL1TARGET).dxp\
    unicows.dxp
DEF1DEPN=\
        $(DEF1EXPORTFILE)\
        makefile.mk

#SHL1VERINFO=$(SHL1TARGET).rc
SHL1OBJS=$(SLOFILES)

#No default libraries
STDSHL=

SHL1STDLIBS=\
        unicows.lib\
        kernel32.lib\
        user32.lib\
        advapi32.lib\
        version.lib\
        msvcrt.lib\
        shlwapi.lib

        
.ENDIF

.INCLUDE : target.mk

