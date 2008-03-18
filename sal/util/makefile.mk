#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.50 $
#
#   last change: $Author: vg $ $Date: 2008-03-18 13:21:43 $
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
UWINAPILIB=     $(LB)$/uwinapi.lib

SHL1STDLIBS=	\
                $(UWINAPILIB)\
                $(ADVAPI32LIB)\
                $(WSOCK32LIB)\
                $(MPRLIB)\
                $(SHELL32LIB)\
                $(COMDLG32LIB)\
                $(USER32LIB)\
                $(OLE32LIB)
.ENDIF # WNT

.IF "$(GUI)"=="UNX"
.IF "$(OS)"=="SOLARIS"
# libposix4.so (SunOS 5.6) <-> librt.so (SunOS >= 5.7)
SHL1STDLIBS= -Bdynamic -ldl -lpthread -lposix4 -lsocket -lnsl
.IF "$(COM)" == "C50"
SHL1STDLIBS+= -z allextract -staticlib=Crun -z defaultextract
.ENDIF # C50
.ENDIF # SOLARIS
.IF "$(OS)"=="IRIX"
SHL1STDLIBS= -lexc
.ENDIF
.ENDIF # UNX

.IF "$(GUI)"=="OS2"
SHL1STDLIBS=pthread.lib
.ENDIF # OS2

# If we compile sal with STLport checking iterators
# we need to link against the STLport
.IF "$(USE_STLP_DEBUG)" != ""
SHL1STDLIBS+=$(LIBSTLPORT)
.ENDIF

#The irony that using the system STL instead of
#stlport requires that we link libsal with the
#LIBSTLPORT alias which is not required when using
#stlport is not lost on me
.IF "$(USE_SYSTEM_STL)"=="YES"
SHL1STDLIBS+=$(LIBSTLPORT)
.ENDIF

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

.IF "$(GUI)"=="WNT"
SHL1DEPN=   $(UWINAPILIB)
.ELSE
SHL1DEPN=
.ENDIF
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

.IF "$(GUI)"=="UNX" || "$(USE_SHELL)"!="4nt"

$(OUT)$/inc$/udkversion.h: 
    echo '#ifndef _SAL_UDKVERSION_H_'           >  $@
    echo '#define _SAL_UDKVERSION_H_'           >> $@
    echo ''                                     >> $@
    echo '#define SAL_UDK_MAJOR "$(UDK_MAJOR)"' >> $@
    echo '#define SAL_UDK_MINOR "$(UDK_MINOR)"' >> $@
    echo '#define SAL_UDK_MICRO "$(UDK_MICRO)"' >> $@
    echo ''                                     >> $@
    echo '#endif'                               >> $@

.ELSE

$(OUT)$/inc$/udkversion.h: 
    echo #ifndef _SAL_UDKVERSION_H_           >  $@
    echo #define _SAL_UDKVERSION_H_           >> $@
    echo.                                     >> $@
    echo #define SAL_UDK_MAJOR "$(UDK_MAJOR)" >> $@
    echo #define SAL_UDK_MINOR "$(UDK_MINOR)" >> $@
    echo #define SAL_UDK_MICRO "$(UDK_MICRO)" >> $@
    echo.                                     >> $@
    echo #endif                               >> $@

.ENDIF
