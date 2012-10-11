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

PRJNAME=odk
TARGET=unowinreg
ENABLE_EXCEPTIONS=TRUE

NO_DEFAULT_STL=TRUE
USE_DEFFILE=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

JAVA_INCLUDES:= -I$(JAVA_HOME)/include

# values taken from set_soenv.in
.IF "$(OS)" == "LINUX"
JAVA_INCLUDES+= -I$(JAVA_HOME)/include/linux
.ELIF "$(OS)" == "FREEBSD"
JAVA_INCLUDES+= -I$(JAVA_HOME)/include/freebsd
JAVA_INCLUDES+= -I$(JAVA_HOME)/include/bsd
JAVA_INCLUDES+= -I$(JAVA_HOME)/include/linux
.ELIF "$(OS)" == "NETBSD"
JAVA_INCLUDES+= -I$(JAVA_HOME)/include/netbsd
.ENDIF

.IF "$(SOLAR_JAVA)"==""
nojava:
    @echo "Not building javaunohelper because Java is disabled"
.ENDIF

.IF "$(OS)" != "WNT"

.IF "$(BUILD_UNOWINREG)" == "YES"

$(BIN)$/unowinreg.dll : unowinreg.cxx
    $(MINGWCXX) -Wall -D_JNI_IMPLEMENTATION_ $(JAVA_INCLUDES) \
        -shared -o $(BIN)$/unowinreg.dll unowinreg.cxx \
            -Wl,--kill-at -lkernel32 -ladvapi32
    $(MINGWSTRIP) $(BIN)$/unowinreg.dll

.ELSE

$(BIN)$/unowinreg.dll : $(TARFILE_LOCATION)/$(UNOWINREG_DLL)
    @@-rm -f $@
    $(GNUCOPY) $< $@

.ENDIF

.ELSE #  "$(OS)" != "WNT"
# Always build unowinreg.dll on windows

# --- Files --------------------------------------------------------

SLOFILES = \
    $(SLO)$/unowinreg.obj
SHL1TARGET=$(TARGET)
SHL1LIBS=$(SLB)$/$(TARGET).lib

#No default libraries
STDSHL=
.IF "$(COM)"=="GCC"
SHL1STDLIBS += -lstdc++
.IF "$(MINGW_GCCLIB_EH)"=="YES"
SHL1STDLIBS += -lgcc_eh
.ENDIF
SHL1STDLIBS += -lgcc -lmingw32 -lmoldname -lmsvcrt
.ELSE
LINKFLAGS+=-MANIFEST:NO
.ENDIF

SHL1STDLIBS +=\
        $(KERNEL32LIB)\
        $(ADVAPI32LIB)

SHL1DEF=$(MISC)$/$(SHL1TARGET).def

DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=$(TARGET).dxp
DEF1DES=unowinreg

.ENDIF #  "$(OS)" != "WNT"

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

