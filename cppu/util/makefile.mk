#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.15 $
#
#   last change: $Author: rt $ $Date: 2007-11-06 15:03:41 $
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

PRJNAME=cppu
TARGET=cppu
USE_DEFFILE=TRUE
ENABLE_EXCEPTIONS=TRUE
NO_BSYMBOLIC=TRUE

.IF "$(OS)" != "WNT"
UNIXVERSIONNAMES=UDK
.ENDIF


# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE : ../source/helper/purpenv/export.mk

# --- Files --------------------------------------------------------

SHL1LIBS= \
    $(SLB)$/cppu_typelib.lib	\
    $(SLB)$/cppu_uno.lib		\
    $(SLB)$/cppu_threadpool.lib	\
    $(SLB)$/cppu_cppu.lib

.IF "$(GUI)" == "WNT" || "$(GUI)"=="OS2"
SHL1TARGET=$(TARGET)$(UDK_MAJOR)
.ELSE
SHL1TARGET= uno_$(TARGET)
.ENDIF

SHL1STDLIBS = $(SALLIB) $(SALHELPERLIB)

SHL1DEPN=
SHL1IMPLIB=i$(TARGET)
.IF "$(OS)"!="FREEBSD"
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
.ENDIF

SHL1VERSIONMAP=$(TARGET).map
SHL1RPATH=URELIB

DEF1NAME=$(SHL1TARGET)


SHL2TARGET  := $(NAMEpurpenv_helper)
DEF2NAME    := $(SHL2TARGET)
.IF "$(GUI)$(COM)"=="WNTGCC" || "$(GUI)"=="OS2"
SHL2VERSIONMAP:=uno_purpenvhelper$(COMID).map
.ELSE
SHL2VERSIONMAP:=$(SHL2TARGET).map
.ENDIF			# "$(GUI)$(COM)"=="WNTGCC"
SHL2DEF     := $(MISC)$/$(SHL2TARGET).def
SHL2IMPLIB  := i$(SHL2TARGET)
SHL2STDLIBS := $(CPPULIB) $(SALHELPERLIB) $(SALLIB)
SHL2RPATH   := URELIB
SHL2OBJS    := \
    $(SLO)$/helper_purpenv_Environment.obj 	\
    $(SLO)$/helper_purpenv_Mapping.obj      \
    $(SLO)$/helper_purpenv_Proxy.obj


# --- Targets ------------------------------------------------------

.PHONY: ALLTAR


ALLTAR:   $(SHL2TARGETN)
    $(MAKE) $(MAKECMDGOALS) -f extra.mk


.INCLUDE :	target.mk


$(SHL2TARGETN): $(SHL1TARGETN)

