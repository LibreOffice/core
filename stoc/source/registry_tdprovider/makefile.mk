#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.14 $
#
#   last change: $Author: rt $ $Date: 2005-09-08 08:03:23 $
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

PRJNAME=	stoc
TARGET = regtypeprov.uno
ENABLE_EXCEPTIONS=TRUE
NO_BSYMBOLIC = TRUE
BOOTSTRAP_SERVICE=TRUE
COMP1TYPELIST = rdbtdp

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
DLLPRE =

# ------------------------------------------------------------------

.INCLUDE :  ..$/cppumaker.mk

SLOFILES=	\
        $(SLO)$/tdprovider.obj	\
        $(SLO)$/td.obj		\
        $(SLO)$/tdef.obj	\
        $(SLO)$/tdenum.obj	\
        $(SLO)$/tdcomp.obj	\
        $(SLO)$/tdconsts.obj	\
        $(SLO)$/tdiface.obj \
        $(SLO)$/tdmodule.obj \
        $(SLO)$/tdprop.obj \
        $(SLO)$/tdservice.obj \
        $(SLO)$/tdsingleton.obj \
        $(SLO)$/rdbtdp_tdenumeration.obj \
        $(SLO)$/functiondescription.obj \
        $(SLO)$/methoddescription.obj \
        $(SLO)$/structtypedescription.obj


SHL1TARGET=	$(TARGET)

SHL1STDLIBS = \
    $(CPPULIB) \
    $(CPPUHELPERLIB) \
    $(REGLIB) \
    $(SALLIB) \
    $(SALHELPERLIB)

SHL1DEPN=
SHL1VERSIONMAP = rdbtdp.map
SHL1IMPLIB=	i$(TARGET)
SHL1LIBS=	$(SLB)$/$(TARGET).lib
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=	$(SHL1TARGET)

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

