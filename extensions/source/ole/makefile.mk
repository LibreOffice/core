#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.20 $
#
#   last change: $Author: vg $ $Date: 2006-09-25 12:45:30 $
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
PRJ=..$/..

# -----------------------------------------------------------------

PRJNAME=extensions
TARGET=oleautobridge.uno

ENABLE_EXCEPTIONS=TRUE

.INCLUDE :  $(PRJ)$/util$/makefile.pmk

LIBTARGET=NO
USE_DEFFILE=YES

USE_DEFFILE=TRUE

INCPRE+= -I$(ATL_INCLUDE)

# --- Settings -----------------------------------------------------

.IF "$(GUI)"=="WNT"

SLOFILES= \
            $(SLO)$/servreg.obj		\
            $(SLO)$/servprov.obj	\
            $(SLO)$/unoobjw.obj		\
            $(SLO)$/oleobjw.obj		\
            $(SLO)$/olethread.obj	\
            $(SLO)$/oledll.obj		\
        $(SLO)$/jscriptclasses.obj	\
        $(SLO)$/ole2uno.obj		\
        $(SLO)$/windata.obj		\
        $(SLO)$/unotypewrapper.obj


SHL1TARGET=$(TARGET)
SHL1STDLIBS=\
        $(SALLIB) 	\
    $(VOSLIB)	\
    $(CPPULIB)	\
    $(CPPUHELPERLIB)	\
    ole32.lib 	\
    uuid.lib 	\
    advapi32.lib	\
    oleaut32.lib

.IF "$(COMEX)"=="8" || "$(COMEX)"=="10"
.IF "$(USE_STLP_DEBUG)" != ""
    SHL1STDLIBS+= $(ATL_LIB)$/atlsd.lib
.ELSE
    SHL1STDLIBS+= $(ATL_LIB)$/atls.lib
.ENDIF
.ENDIF

SHL1LIBS=
SHL1OBJS=$(SLOFILES)

DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=$(SHL1TARGET).dxp

.ENDIF

#----------------------------------------------------------------

.INCLUDE :  target.mk
