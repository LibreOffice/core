#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

PRJ=..$/..$/..

PRJNAME=odk
TARGET=unowinreg
ENABLE_EXCEPTIONS=TRUE

NO_DEFAULT_STL=TRUE
USE_DEFFILE=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

JAVA_INCLUDES:= -I$(JAVA_HOME)/include

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

