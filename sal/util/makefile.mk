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

PRJ=..

PRJNAME=sal
TARGET=sal

NO_BSYMBOLIC=TRUE
NO_DEFAULT_STL=TRUE

USE_LDUMP2=TRUE
USE_DEFFILE=TRUE

.IF "$(GUI)"=="UNX"
TARGETTYPE=CUI
.ENDIF # UNX

UNIXVERSIONNAMES=UDK

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

CFLAGS+= $(LFS_CFLAGS)
CXXFLAGS+= $(LFS_CFLAGS)

.IF "$(depend)" == ""

# --- Files --------------------------------------------------------

# disable check for PIC code as it would complain about 
# hand coded assembler
CHECKFORPIC=

.IF "$(header)" == ""

LIB1TARGET=$(SLB)$/$(TARGET).lib
LIB1FILES=	\
    $(SLB)$/oslall.lib	\
    $(SLB)$/cpposl.lib	\
    $(SLB)$/cpprtl.lib	\
    $(SLB)$/textenc.lib 

LIB3TARGET=$(LB)$/a$(TARGET).lib
LIB3ARCHIV=$(LB)$/lib$(TARGET)$(DLLPOSTFIX).a
LIB3FILES=	\
    $(LB)$/oslall.lib	\
    $(LB)$/cpposl.lib	\
    $(LB)$/cpprtl.lib	\
    $(LB)$/textenc.lib


.IF "$(GUI)" == "WNT" || "$(GUI)"=="OS2"
SHL1TARGET= $(TARGET)
.ELSE
SHL1TARGET= uno_$(TARGET)
.ENDIF
SHL1IMPLIB= i$(TARGET)
SHL1VERSIONMAP=	$(TARGET).map
SHL1RPATH=URELIB

.IF "$(GUI)"=="WNT"

.IF "$(COM)"=="GCC"
UWINAPILIB=     -luwinapi
.ELSE
UWINAPILIB=     $(LB)$/uwinapi.lib
.ENDIF

SHL1STDLIBS=	\
                $(UWINAPILIB)\
                $(ADVAPI32LIB)\
                $(WSOCK32LIB)\
                $(MPRLIB)\
                $(SHELL32LIB)\
                $(COMDLG32LIB)\
                $(USER32LIB)\
                $(OLE32LIB)
.IF "$(COM)"=="GCC"
MINGWSSTDOBJ=
MINGWSSTDENDOBJ=
.ENDIF
.ENDIF # WNT

.IF "$(GUI)"=="UNX"
.IF "$(OS)"=="SOLARIS"
# libposix4.so (SunOS 5.6) <-> librt.so (SunOS >= 5.7)
SHL1STDLIBS= -Bdynamic -ldl -lpthread -lposix4 -lsocket -lnsl
.IF "$(COM)" == "C50"
SHL1STDLIBS+= -z allextract -staticlib=Crun -z defaultextract
.ENDIF # C50
.ENDIF # SOLARIS
.ENDIF # UNX

.IF "$(GUI)"=="OS2"
SHL1STDLIBS=pthread.lib
.ENDIF # OS2

.IF "$(OS)"=="MACOSX"
SHL1STDLIBS+=-framework CoreFoundation -framework Carbon
.ENDIF

.IF "$(OS)" == "LINUX"
.IF "$(PAM_LINK)" == "YES"
SHL1STDLIBS+=-lpam
.ENDIF
.IF "$(CRYPT_LINK)" == "YES"
SHL1STDLIBS+=-lcrypt
.ENDIF
.ENDIF

SHL1LIBS+=$(SLB)$/$(TARGET).lib

.IF "$(linkinc)" != ""
SHL11FILE=$(MISC)$/sal.slo
.ELSE
.IF "$(GUI)"=="UNX"
SHL1OBJS=
.ELSE
.IF "$(GUI)$(COM)"!="WNTGCC"
SHL1OBJS= \
    $(SLO)$/dllentry.obj
.ENDIF # WNTGCC
.ENDIF # UNX
.ENDIF # lincinc

SHL1DEPN=
SHL1DEF=    $(MISC)$/$(SHL1TARGET).def

DEF1NAME= $(SHL1TARGET)

# --- Coverage -----------------------------------------------------
# LLA: 20040304 The follows lines are an additional which is only need if we run
#               coverage tests. For normal test runs this feature is not used.
#               For more information about coverage tests see:
#               http://gcc.gnu.org/onlinedocs/gcc-3.0/gcc_8.html
#
#               Why this additional?
#               Anybody has decide to link sal with g++ instead of gcc.
#
.IF "$(TESTCOVERAGE)"!=""
.IF "$(GUI)"=="UNX"
.IF "$(COM)"=="GCC"
.IF "$(OS)"=="LINUX"
SHL1STDLIBS+=-lgcc
.ENDIF
.ENDIF
.ENDIF
.ENDIF

SHL4DEPN+=$(SHL1TARGETN)
SHL4LIBS=$(SLB)$/textenc_tables.lib
SHL4TARGET=uno_sal_textenc
SHL4STDLIBS=$(SALLIB)
SHL4VERSIONMAP=saltextenc.map

SHL4DEF=$(MISC)$/$(SHL4TARGET).def
DEF4NAME=$(SHL4TARGET)

# --- Targets ------------------------------------------------------

.ENDIF # $(header) != ""
.ENDIF # $(depend) != ""

.INCLUDE :  target.mk

.IF "$(SHL1TARGETN)" != ""
$(SHL1TARGETN) : $(OUT)$/inc$/udkversion.h
.ENDIF # "$(SHL1TARGETN)" != ""

$(OUT)$/inc$/udkversion.h: 
    echo '#ifndef _SAL_UDKVERSION_H_'           >  $@
    echo '#define _SAL_UDKVERSION_H_'           >> $@
    echo ''                                     >> $@
    echo '#define SAL_UDK_MAJOR "$(UDK_MAJOR)"' >> $@
    echo '#define SAL_UDK_MINOR "$(UDK_MINOR)"' >> $@
    echo '#define SAL_UDK_MICRO "$(UDK_MICRO)"' >> $@
    echo ''                                     >> $@
    echo '#endif'                               >> $@
