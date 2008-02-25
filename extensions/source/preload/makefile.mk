#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.7 $
#
#   last change: $Author: obo $ $Date: 2008-02-25 15:33:40 $
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
TARGET=preload
ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :	$(PRJ)$/util$/makefile.pmk

# --- defines ------------------------------------------------------

CDEFS+=-DCOMPMOD_NAMESPACE=preload
CDEFS+=-DCOMPMOD_RESPREFIX=preload

# --- Files --------------------------------------------------------

SLOFILES=           $(SLO)$/services.obj\
                    $(SLO)$/modulepreload.obj\
                    $(SLO)$/preloadservices.obj\
                    $(SLO)$/oemwiz.obj

SRS1NAME=$(TARGET)
SRC1FILES=           preload.src

RESLIB1NAME=preload
RESLIB1IMAGES=$(PRJ)$/res
RESLIB1SRSFILES= $(SRS)$/preload.srs

SHL1TARGET= $(TARGET)$(DLLPOSTFIX)
SHL1VERSIONMAP= $(TARGET).map

SHL1STDLIBS= \
        $(SVXLIB)	\
        $(SFXLIB)   \
        $(SVTOOLLIB)\
        $(VCLLIB)    \
        $(SVLLIB)	\
        $(TOOLSLIB)  \
        $(COMPHELPERLIB)     \
        $(CPPUHELPERLIB) 	 \
        $(CPPULIB)   \
        $(SALLIB)


SHL1LIBS=		$(SLB)$/$(TARGET).lib
SHL1IMPLIB=		i$(TARGET)
SHL1DEPN=		$(SHL1LIBS)
SHL1DEF=		$(MISC)$/$(SHL1TARGET).def

DEF1NAME=		$(SHL1TARGET)

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

