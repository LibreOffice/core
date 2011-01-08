#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

PRJ=..$/..$/..

PRJNAME=vcl
TARGET=salgdi

.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile2.pmk

# --- Files --------------------------------------------------------

.IF "$(GUIBASE)"!="unx"

dummy:
    @echo "Nothing to build for GUIBASE $(GUIBASE)"

.ELSE		# "$(GUIBASE)"!="unx"

SLOFILES=	\
        $(SLO)$/salgdi2.obj		\
        $(SLO)$/salbmp.obj		\
        $(SLO)$/salgdi.obj		\
        $(SLO)$/salvd.obj		\
        $(SLO)$/dtint.obj		\
        $(SLO)$/salcvt.obj		\
        $(SLO)$/salgdi3.obj		\
        $(SLO)$/gcach_xpeer.obj		\
        $(SLO)$/xrender_peer.obj	\
        $(SLO)$/pspgraphics.obj
        
EXCEPTIONSFILES=\
        $(SLO)$/salgdi.obj		\
        $(SLO)$/salbmp.obj		\
        $(SLO)$/salgdi3.obj		\
        $(SLO)$/salcvt.obj

SLOFILES+=$(SLO)$/salprnpsp.obj
EXCEPTIONSFILES+=$(SLO)$/salprnpsp.obj

.IF "$(OS)"=="SOLARIS"
SLOFILES+=$(SLO)$/cdeint.obj
ENVCFLAGS+=-DUSE_CDE
.ENDIF

.IF "$(XRENDER_LINK)" == "YES"
CFLAGS+=-DXRENDER_LINK
.ENDIF

.IF "$(ENABLE_GRAPHITE)" == "TRUE"
CFLAGS+=-DENABLE_GRAPHITE
.ENDIF

.ENDIF	# "$(GUIBASE)"!="unx"

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

.INCLUDE :  $(PRJ)$/util$/target.pmk

ALLTAR : $(MACOSXRC)

XSALSETLIBNAME=$(DLLPRE)spa$(DLLPOSTFIX)$(DLLPOST)

$(INCCOM)$/rtsname.hxx:
    @rm -f $(INCCOM)$/rtsname.hxx ; \
        echo "#define _XSALSET_LIBNAME "\"$(XSALSETLIBNAME)\" > $(INCCOM)$/rtsname.hxx

$(SLO)$/salpimpl.obj : $(INCCOM)$/rtsname.hxx
$(SLO)$/salprnpsp.obj : $(INCCOM)$/rtsname.hxx

