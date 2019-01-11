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

.IF "$(OS)" != "MACOSX"
UNIXVERSIONNAMES=UDK
.ENDIF

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

#.IF "$(GUI)"=="UNX"
#LIB1FILES+=$(SLB)$/systoolsunx.lib
#.ENDIF # UNX


LIB3TARGET=$(LB)$/a$(TARGET).lib
LIB3ARCHIV=$(LB)$/lib$(TARGET)$(DLLPOSTFIX).a
LIB3FILES=	\
    $(LB)$/oslall.lib	\
    $(LB)$/cpposl.lib	\
    $(LB)$/cpprtl.lib	\
    $(LB)$/textenc.lib

#.IF "$(GUI)"=="UNX"
#LIB3FILES+=$(LB)$/systoolsunx.lib
#.ENDIF # UNX

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
SHL1STDLIBS=-lmmap -lpthread
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

# #i105898# required for LD_PRELOAD libsalalloc_malloc.so
#           if sal is linked with -Bsymbolic-functions
.IF "$(HAVE_LD_BSYMBOLIC_FUNCTIONS)" == "TRUE"
SHL1LINKFLAGS+=-Wl,--dynamic-list=salalloc.list
.ENDIF # .IF "$(HAVE_LD_BSYMBOLIC_FUNCTIONS)" == "TRUE"

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

#
# This part builds a tiny extra lib,
# containing an alloc.c which uses system 
# heap instead of our own mem management. 
# This is e.g. useful for proper valgrinding
# the office.
#
.IF "$(OS)"=="LINUX"

TARGET2 = salalloc_malloc
SHL2TARGET= $(TARGET2)
SHL2IMPLIB= i$(TARGET2)
SHL2VERSIONMAP=	salalloc.map

SHL2LIBS+=$(SLB)$/SYSALLOC_cpprtl.lib

.ENDIF # .IF "$(OS)"=="LINUX"

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
