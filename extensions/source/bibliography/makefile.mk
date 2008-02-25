#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.22 $
#
#   last change: $Author: obo $ $Date: 2008-02-25 15:32:34 $
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

PRJNAME=extensions
TARGET=bib
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :	$(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SLOFILES=			$(SLO)$/formcontrolcontainer.obj \
                    $(SLO)$/loadlisteneradapter.obj \
                    $(SLO)$/bibconfig.obj \
                    $(SLO)$/bibcont.obj \
                    $(SLO)$/bibload.obj \
                    $(SLO)$/bibmod.obj \
                    $(SLO)$/general.obj \
                    $(SLO)$/framectr.obj \
                    $(SLO)$/bibview.obj \
                    $(SLO)$/bibbeam.obj  \
                    $(SLO)$/toolbar.obj  \
                    $(SLO)$/datman.obj	\


SRS1NAME=$(TARGET)
SRC1FILES=			bib.src	 \
                    datman.src \
                    sections.src \
                    menu.src	 \
                    toolbar.src

RESLIB1NAME=bib
RESLIB1SRSFILES= $(SRS)$/bib.srs
RESLIB1DEPN= bib.src bib.hrc


SHL1STDLIBS= \
        $(SOTLIB)           \
        $(SVTOOLLIB) \
        $(TKLIB) \
        $(VCLLIB) \
        $(SVLLIB) \
        $(UNOTOOLSLIB) \
        $(TOOLSLIB) \
        $(DBTOOLSLIB) \
        $(COMPHELPERLIB) \
        $(CPPUHELPERLIB) \
        $(CPPULIB) \
        $(SALLIB)


SHL1TARGET= $(TARGET)$(DLLPOSTFIX)
SHL1LIBS=       $(SLB)$/$(TARGET).lib
SHL1IMPLIB=		i$(TARGET)
SHL1DEPN=		$(SHL1LIBS)
SHL1DEF=		$(MISC)$/$(SHL1TARGET).def
SHL1VERSIONMAP=exports.map
DEF1NAME=		$(SHL1TARGET)

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

