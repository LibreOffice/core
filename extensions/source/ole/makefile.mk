#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.26 $
#
#   last change: $Author: ihi $ $Date: 2008-02-04 12:58:39 $
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
TARGET2=oleautobridge2.uno

ENABLE_EXCEPTIONS=TRUE

.INCLUDE :  $(PRJ)$/util$/makefile.pmk

LIBTARGET=NO
USE_DEFFILE=YES

USE_DEFFILE=TRUE

INCPRE+= $(foreach,i,$(ATL_INCLUDE) -I$(i))

# --- Settings -----------------------------------------------------

.IF "$(GUI)" == "WNT" && "$(DISABLE_ATL)"==""

.IF "$(USE_STLP_DEBUG)"!=""
CDEFS+=-D_DEBUG
.ENDIF # "$(USE_STLP_DEBUG)"!=""

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

SECOND_BUILD=OWNGUID
OWNGUID_SLOFILES=$(SLOFILES)
OWNGUIDCDEFS+= -DOWNGUID

# the original library
SHL1TARGET=$(TARGET)
SHL1STDLIBS=\
        $(SALLIB) 	\
    $(VOSLIB)	\
    $(CPPULIB)	\
    $(CPPUHELPERLIB)	\
    $(OLE32LIB) 	\
    $(UUIDLIB) 	\
    $(ADVAPI32LIB)	\
    $(OLEAUT32LIB)

.IF "$(COM)"=="MSC"
.IF "$(WINDOWS_VISTA_PSDK)"!="" || "$(CCNUMVER)"<="001399999999"
.IF "$(USE_STLP_DEBUG)" != ""
    SHL1STDLIBS+= $(ATL_LIB)$/atlsd.lib
.ELSE
    SHL1STDLIBS+= $(ATL_LIB)$/atls.lib
.ENDIF
.ENDIF # "$(WINDOWS_VISTA_PSDK)"!="" || "$(CCNUMVER)"<="001399999999"
.ENDIF # "$(COM)"=="MSC"

SHL1LIBS=
SHL1OBJS=$(SLOFILES)

DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=$(SHL1TARGET).dxp

# the second library
SHL2TARGET=$(TARGET2)
SHL2STDLIBS=\
        $(SALLIB) 	\
    $(VOSLIB)	\
    $(CPPULIB)	\
    $(CPPUHELPERLIB)	\
    $(OLE32LIB) 	\
    $(UUIDLIB) 	\
    $(ADVAPI32LIB)	\
    $(OLEAUT32LIB)

.IF "$(COM)"=="MSC"
.IF "$(WINDOWS_VISTA_PSDK)"!="" || "$(CCNUMVER)"<="001399999999"
.IF "$(USE_STLP_DEBUG)" != ""
    SHL2STDLIBS+= $(ATL_LIB)$/atlsd.lib
.ELSE
    SHL2STDLIBS+= $(ATL_LIB)$/atls.lib
.ENDIF
.ENDIF # "$(WINDOWS_VISTA_PSDK)"!="" || "$(CCNUMVER)"<="001399999999"
.ENDIF # "$(COM)"=="MSC"

SHL2LIBS=
SHL2OBJS=$(REAL_OWNGUID_SLOFILES)

DEF2NAME=$(SHL2TARGET)
DEF2EXPORTFILE=$(TARGET).dxp

.ENDIF

#----------------------------------------------------------------

.INCLUDE :  target.mk

