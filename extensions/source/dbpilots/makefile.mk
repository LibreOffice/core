#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.9 $
#
#   last change: $Author: obo $ $Date: 2008-02-25 15:32:47 $
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
TARGET=dbp
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :	$(PRJ)$/util$/makefile.pmk

# --- defines ------------------------------------------------------

CDEFS+=-DCOMPMOD_NAMESPACE=dbp

# --- Files --------------------------------------------------------

CXXFILES=			dbptools.cxx	\
                    gridwizard.cxx	\
                    listcombowizard.cxx	\
                    optiongrouplayouter.cxx	\
                    commonpagesdbp.cxx	\
                    groupboxwiz.cxx	\
                    wizardservices.cxx	\
                    controlwizard.cxx	\
                    moduledbp.cxx	\
                    dbpservices.cxx

SLOFILES=			$(SLO)$/dbptools.obj	\
                    $(SLO)$/gridwizard.obj	\
                    $(SLO)$/listcombowizard.obj	\
                    $(SLO)$/optiongrouplayouter.obj	\
                    $(SLO)$/commonpagesdbp.obj	\
                    $(SLO)$/groupboxwiz.obj	\
                    $(SLO)$/wizardservices.obj	\
                    $(SLO)$/controlwizard.obj	\
                    $(SLO)$/moduledbp.obj	\
                    $(SLO)$/dbpservices.obj


SRS1NAME=$(TARGET)
SRC1FILES=			gridpages.src	\
                    commonpagesdbp.src	\
                    dbpilots.src	\
                    groupboxpages.src	\
                    listcombopages.src

RESLIB1NAME=dbp
RESLIB1SRSFILES= $(SRS)$/dbp.srs

SHL1TARGET= $(TARGET)$(DLLPOSTFIX)
SHL1VERSIONMAP= $(TARGET).map

SHL1STDLIBS= \
        $(SVTOOLLIB) \
        $(VCLLIB) \
        $(TOOLSLIB) \
        $(SVLLIB) \
        $(SFXLIB)		\
        $(DBTOOLSLIB) \
        $(COMPHELPERLIB) \
        $(CPPUHELPERLIB) \
        $(CPPULIB) \
        $(SALLIB)

SHL1LIBS=		$(SLB)$/$(TARGET).lib
SHL1IMPLIB=		i$(TARGET)
SHL1DEPN=		$(SHL1LIBS)
SHL1DEF=		$(MISC)$/$(SHL1TARGET).def

DEF1NAME=		$(SHL1TARGET)

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk
