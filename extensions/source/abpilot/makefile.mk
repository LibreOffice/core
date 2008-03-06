#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.13 $
#
#   last change: $Author: kz $ $Date: 2008-03-06 18:38:45 $
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
PRJINC=..$/inc

PRJNAME=extensions
TARGET=abp
USE_DEFFILE=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :	$(PRJ)$/util$/makefile.pmk

# --- defines ------------------------------------------------------

CDEFS+=-DCOMPMOD_NAMESPACE=abp
CDEFS+=-DCOMPMOD_RESPREFIX=abp

# --- Files --------------------------------------------------------

EXCEPTIONSFILES=	\
        $(SLO)$/fieldmappingimpl.obj	\
        $(SLO)$/datasourcehandling.obj	\
        $(SLO)$/admininvokationimpl.obj	\
        $(SLO)$/unodialogabp.obj	\
        $(SLO)$/moduleabp.obj	\
        $(SLO)$/abpservices.obj \
        $(SLO)$/typeselectionpage.obj	\

SLOFILES=	\
        $(SLO)$/fieldmappingpage.obj	\
        $(SLO)$/abpfinalpage.obj	\
        $(SLO)$/tableselectionpage.obj	\
        $(SLO)$/admininvokationpage.obj	\
        $(SLO)$/abspage.obj	\
        $(SLO)$/abspilot.obj \
        $(EXCEPTIONSFILES)


.IF "$(WITH_MOZILLA)" != "NO"
.IF "$(SYSTEM_MOZILLA)" != "YES"
CDEFS+=-DWITH_MOZILLA
.ENDIF
.ENDIF

SRS1NAME=$(TARGET)
SRC1FILES=			abspilot.src

RESLIB1NAME=abp
RESLIB1IMAGES=$(PRJ)$/res
RESLIB1SRSFILES= $(SRS)$/abp.srs

SHL1TARGET= $(TARGET)$(DLLPOSTFIX)
SHL1VERSIONMAP= $(TARGET).map

SHL1STDLIBS= \
        $(VCLLIB)			\
        $(SALLIB)			\
        $(TOOLSLIB)			\
        $(CPPULIB) 			\
        $(SVTOOLLIB)		\
        $(SVLLIB)			\
        $(SFXLIB)			\
        $(CPPUHELPERLIB)	\
        $(COMPHELPERLIB)	\
        $(TKLIB)			\
        $(UNOTOOLSLIB)		\
        $(SVXLIB)

SHL1LIBS=		$(SLB)$/$(TARGET).lib
SHL1IMPLIB=		i$(TARGET)
SHL1DEPN=		$(SHL1LIBS)
SHL1DEF=		$(MISC)$/$(SHL1TARGET).def

DEF1NAME=		$(SHL1TARGET)
DEF1EXPORTFILE=	exports.dxp

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk
