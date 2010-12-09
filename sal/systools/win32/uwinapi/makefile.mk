#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
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
.IF "$(COM)"=="GCC"
CFLAGSCXX+=-Wno-unused-parameter -Wno-return-type
.ENDIF

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
        $(MINGW_LIBGCC)
MINGWSSTDOBJ=
MINGWSSTDENDOBJ=
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
