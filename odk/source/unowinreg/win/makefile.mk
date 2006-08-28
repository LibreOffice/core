#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.6 $
#
#   last change: $Author: ihi $ $Date: 2006-08-28 11:29:52 $
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
.IF "$(JDK)" != "gcj"
.IF "$(OS)" == "LINUX"
JAVA_INCLUDES+= -I$(JAVA_HOME)/include/linux
.ELIF "$(OS)" == "FREEBSD"
JAVA_INCLUDES+= -I$(JAVA_HOME)/include/freebsd
JAVA_INCLUDES+= -I$(JAVA_HOME)/include/bsd
JAVA_INCLUDES+= -I$(JAVA_HOME)/include/linux
.ELIF "$(OS)" == "NETBSD"
JAVA_INCLUDES+= -I$(JAVA_HOME)/include/netbsd
.ELIF "$(OS)" == "IRIX"
JAVA_INCLUDES+= -I$(JAVA_HOME)/include/solaris
.ENDIF
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

$(BIN)$/unowinreg.dll : $(SOLARVERSION)$/$(INPATH)$/bin$(UPDMINOREXT)$/unowinreg.dll
    +-rm -f $@ >& $(NULLDEV)
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

SHL1STDLIBS +=\
        kernel32.lib\
        advapi32.lib
        
SHL1DEF=$(MISC)$/$(SHL1TARGET).def

DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=$(TARGET).dxp
DEF1DES=unowinreg

NO_SHL1DESCRIPTION=TRUE

.ENDIF #  "$(OS)" != "WNT"

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

