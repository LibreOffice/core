#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.16 $
#
#   last change: $Author: rt $ $Date: 2008-03-12 07:38:41 $
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

CFLAGS+= $(LFS_CFLAGS)
CXXFLAGS+= $(LFS_CFLAGS)

# --- Targets ----------------------------------


.IF "$(GUI)"=="WNT"

.IF "$(COMEX)"=="9"
.IF "$(PSDK_HOME)"!=""
# Since the 02/2003 PSDK the "new" linker is needed here.
LINK=$(WRAPCMD) "$(PSDK_HOME)$/Bin$/Win64$/LINK.EXE"
.ENDIF
.ENDIF

SLOFILES=\
        $(SLO)$/CheckTokenMembership.obj\
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
        $(SLO)$/ResolveUnicows.obj\
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
        $(SLO)$/PathStripToRootW.obj\
        $(SLO)$/SHCreateItemFromParsingName.obj
        
SHL1TARGET=$(TARGET)
SHL1IMPLIB=$(SHL1TARGET)
SHL1DEF=$(MISC)/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)
.IF "$(COM)"=="GCC"
DEF1EXPORTFILE=\
    $(SHL1TARGET)_mingw.dxp\
    unicows_mingw.dxp
.ELSE
DEF1EXPORTFILE=\
    $(SHL1TARGET).dxp\
    unicows.dxp
.ENDIF
DEF1DEPN=\
        $(DEF1EXPORTFILE)\
        makefile.mk

#SHL1VERINFO=$(SHL1TARGET).rc
SHL1OBJS=$(SLOFILES)

#No default libraries
STDSHL=

.IF "$(COM)"=="GCC"
SHL1STDLIBS=\
        -lmingw32 \
        -lgcc
.ELSE
SHL1STDLIBS=\
        unicows.lib
.ENDIF

SHL1STDLIBS+=\
        $(KERNEL32LIB)\
        $(USER32LIB)\
        $(ADVAPI32LIB)\
        $(VERSIONLIB)\
        $(LIBCMT)\
        $(SHLWAPILIB)


.ENDIF

.IF "$(COM)"=="GCC"
ALL: ALLTAR $(LB)$/libuwinapi.a

$(LB)$/libuwinapi.a: $(MISC)$/uwinapi.def
    dlltool --dllname uwinapi.dll --input-def=$(MISC)$/uwinapi.def --kill-at --output-lib=$(LB)$/libuwinapi.a
.ENDIF

.INCLUDE : target.mk
