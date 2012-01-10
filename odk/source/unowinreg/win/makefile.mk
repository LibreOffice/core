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

PRJNAME=odk
TARGET=unowinreg
ENABLE_EXCEPTIONS=TRUE

NO_DEFAULT_STL=TRUE
NO_BSYMBOLIC=TRUE
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
        -I$(PRJ)/inc/pch -shared -o $(BIN)$/unowinreg.dll unowinreg.cxx \
            -Wl,--kill-at -lkernel32 -ladvapi32
    $(MINGWSTRIP) $(BIN)$/unowinreg.dll

.ELSE

$(BIN)$/unowinreg.dll : $(SOLARVERSION)$/$(INPATH)$/bin$(UPDMINOREXT)$/unowinreg.dll
    @@-rm -f $@
    $(GNUCOPY) $< $@

.ENDIF

.ELSE #  "$(OS)" != "WNT"
# Always build unowinreg.dll on windows

# --- Files --------------------------------------------------------

LINKFLAGS+=-MANIFEST:NO
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

