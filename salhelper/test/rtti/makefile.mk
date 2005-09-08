#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: rt $ $Date: 2005-09-08 17:02:46 $
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

PRJNAME=	salhelper
TARGET=		rtti
TARGET1=samplelibrtti
LIBTARGET=NO
TARGETTYPE=CUI


NO_BSYMBOLIC=	TRUE
ENABLE_EXCEPTIONS=TRUE

USE_DEFFILE=	TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

#RTTI on
.IF "$(OS)" == "WNT"
CFLAGS+= -GR
.ENDIF

SLOFILES=	\
        $(SLO)$/samplelibrtti.obj

LIB1TARGET=$(SLB)$/$(TARGET1).lib
LIB1OBJFILES= \
        $(SLO)$/samplelibrtti.obj

SHL1TARGET=	$(TARGET1)

SHL1STDLIBS= \
        $(CPPULIB)		\
        $(CPPUHELPERLIB)	\
        $(SALLIB)


SHL1DEPN=
SHL1IMPLIB=	i$(TARGET1)
SHL1LIBS=	$(SLB)$/$(TARGET1).lib
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def
DEF1EXPORTFILE=	exports.dxp

DEF1NAME=	$(SHL1TARGET)
# generate exports ------------------------------------------------
#DEF1DEPN=      $(MISC)$/$(SHL1TARGET).flt
#DEFLIB1NAME=   $(TARGET)
#-----------------------------------------------------------------


.IF "$(OS)$(CPU)"=="SOLARISS"
SHL1VERSIONMAP=	sols.map
.ELIF "$(OS)$(CPU)"=="LINUXI"
SHL1VERSIONMAP= lngi.map
.ELIF "$(OS)$(CPU)$(COMNAME)" == "GCCFREEBSDIgcc2"
SHL1VERSIONMAP= gcc2_freebsd_intel.map
.ELIF "$(OS)$(CPU)$(COMNAME)" == "GCCFREEBSDIgcc3"
SHL1VERSIONMAP= gcc3_freebsd_intel.map
.ENDIF


# ------------------------------------------------------------------

APP1NOSAL=TRUE

APP1TARGET=	$(TARGET)

APP1OBJS=	$(OBJ)$/rttitest.obj

APP1STDLIBS= \
    $(SALLIB) \
    $(CPPUHELPERLIB) \
    $(CPPULIB)

#LIBCIMT=msvcrtd.lib

.IF "$(OS)" == "WNT"
APP1STDLIBS+=	$(LB)$/isamplelibrtti.lib
.ELSE
APP1STDLIBS+=	-lsamplelibrtti
.ENDIF 


.IF "$(GUI)"=="WNT"
APP1STDLIBS += $(LIBCIMT)
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

$(MISC)$/$(SHL1TARGET).flt : makefile.mk
       +echo   _TI2       >$@
       +echo   _TI1      >>$@

