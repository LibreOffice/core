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

PRJ=..

PRJNAME=linguistic
TARGET=lng
ENABLE_EXCEPTIONS=sal_True

#----- Settings ---------------------------------------------------------

.INCLUDE : settings.mk

# --- Files --------------------------------------------------------

SLOFILES = \
        $(SLO)$/convdiclist.obj\
        $(SLO)$/convdic.obj\
        $(SLO)$/convdicxml.obj\
        $(SLO)$/dicimp.obj\
        $(SLO)$/dlistimp.obj\
        $(SLO)$/hhconvdic.obj\
        $(SLO)$/hyphdsp.obj\
        $(SLO)$/hyphdta.obj\
        $(SLO)$/iprcache.obj\
        $(SLO)$/lngopt.obj\
        $(SLO)$/lngprophelp.obj\
        $(SLO)$/lngreg.obj\
        $(SLO)$/lngsvcmgr.obj\
        $(SLO)$/misc.obj\
        $(SLO)$/misc2.obj\
        $(SLO)$/spelldsp.obj\
        $(SLO)$/spelldta.obj\
        $(SLO)$/thesdsp.obj\
        $(SLO)$/thesdta.obj\
        $(SLO)$/gciterator.obj

SHL1TARGET= $(TARGET)$(DLLPOSTFIX)

SHL1STDLIBS= \
        $(CPPULIB) 	 \
        $(CPPUHELPERLIB) 	 \
        $(COMPHELPERLIB)   \
        $(TOOLSLIB)		\
        $(I18NISOLANGLIB) \
        $(SVLLIB)	\
        $(SALLIB)		\
        $(XMLOFFLIB)    \
        $(UCBHELPERLIB)	\
        $(UNOTOOLSLIB)  \
        $(ICUUCLIB)
        
# build DLL
SHL1LIBS=		$(SLB)$/$(TARGET).lib
SHL1DEPN=		$(SLB)$/$(TARGET).lib
SHL1DEF=		$(MISC)$/$(SHL1TARGET).def
SHL1IMPLIB= ilng

# build DEF file
DEF1NAME	=$(SHL1TARGET)
DEF1DEPN    =$(MISC)$/$(SHL1TARGET).flt
DEFLIB1NAME =$(TARGET)
DEF1DES     =Linguistic main DLL


.IF "$(GUI)"=="WNT"
DEF1EXPORT1 = component_getFactory
DEF1EXPORT2 = component_getImplementationEnvironment
.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE : target.mk


$(MISC)$/$(SHL1TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo lcl > $@
    @echo component >> $@
    @echo __CT >> $@


ALLTAR : $(MISC)/lng.component

$(MISC)/lng.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        lng.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt lng.component
