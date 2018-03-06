#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



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
        $(SLO)$/ResolveThunk.obj\
        $(SLO)$/SHCreateItemFromParsingName.obj\
        $(SLO)$/snprintf.obj\
        $(SLO)$/snwprintf.obj


.IF "$(CPUNAME)"=="INTEL"
SLOFILES+=\
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
        $(SLO)$/ResolveUnicows.obj\
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
.ENDIF
        
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
    $(SHL1TARGET)64.dxp
.IF "$(CPUNAME)"=="INTEL"
DEF1EXPORTFILE+=\
    $(SHL1TARGET).dxp\
    unicows.dxp
.ENDIF
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
