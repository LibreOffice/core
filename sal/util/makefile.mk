#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.31 $
#
#   last change: $Author: hr $ $Date: 2003-07-16 17:23:32 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#
#*************************************************************************

PRJ=..

$(PRJPCH)=

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


SHL1TARGET= $(TARGET)
SHL1IMPLIB= i$(TARGET)

SHL1VERSIONMAP=	$(TARGET).map

.IF "$(GUI)"=="WNT"
.IF "$(COM)"!="GCC"
SHL1STDLIBS=	\
                uwinapi.lib\
                advapi32.lib\
                wsock32.lib\
                mpr.lib\
                shell32.lib\
                comdlg32.lib\
                user32.lib\
                ole32.lib
.ELSE
SHL1STDLIBS= -luwinapi.lib -ladvapi32 -lwsock32 -lmpr -lole32
.ENDIF # GCC
.ENDIF # WNT

.IF "$(GUI)"=="MAC"
SHL1STDLIBS=-L$(shell $(UNIX2MACPATH) $(MW_HOME)$/Metrowerks\ CodeWarrior$/MacOS\ Support$/OpenTransport$/Open\ Tpt\ Client\ Developer$/PPC\ Libraries) \
  -weakimport -lOpenTransportLib -weakimport -lOpenTptInternetLib \
  -lOpenTransportExtnPPC.o -lOpenTptInetPPC.o
SHL1STDLIBS+=-init InitLibrary -term ExitLibrary
.ENDIF # MAC

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

.IF "$(OS)"=="MACOSX"
SHL1STDLIBS+=-lstlport_gcc -framework CoreFoundation
.ENDIF

SHL1LIBS+=$(SLB)$/$(TARGET).lib

.IF "$(linkinc)" != ""
SHL11FILE=$(MISC)$/sal.slo
.ELSE
.IF "$(GUI)"=="UNX"
SHL1OBJS=
.ELSE
.IF "$(GUI)"=="MAC"
SHL1OBJS= \
    $(OBJ)$/dllentry.obj
.ELSE
SHL1OBJS= \
    $(SLO)$/dllentry.obj
.ENDIF # MAC
.ENDIF # UNX
.ENDIF # lincinc

SHL1DEPN=
SHL1DEF=    $(MISC)$/$(SHL1TARGET).def

DEF1NAME= $(SHL1TARGET)

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

