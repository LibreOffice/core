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

# tk.dxp should contain all c functions that have to be exported. MT 2001/11/29

PRJ=..

PRJNAME=toolkit
TARGET=tk
TARGET2=tka
USE_DEFFILE=TRUE

# --- Settings -----------------------------------------------------------

.INCLUDE :	settings.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk


# --- Allgemein ----------------------------------------------------------

# ========================================================================
# = tk lib: the "classic" toolkit library

LIB1TARGET= $(SLB)$/$(TARGET).lib
LIB1FILES=	$(SLB)$/awt.lib \
            $(SLB)$/tree.lib \
            $(SLB)$/grid.lib \
            $(SLB)$/controls.lib \
            $(SLB)$/helper.lib\
            $(SLB)$/layout-core.lib \
            $(SLB)$/layout-vcl.lib

SHL1TARGET= tk$(DLLPOSTFIX)
SHL1IMPLIB= itk
SHL1USE_EXPORTS=name

SHL1STDLIBS=\
        $(VCLLIB)			\
        $(UNOTOOLSLIB)		\
        $(TOOLSLIB)			\
        $(COMPHELPERLIB)	\
        $(CPPUHELPERLIB)	\
        $(CPPULIB)			\
        $(SALLIB)

SHL1LIBS=	$(LIB1TARGET)
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def
SHL1DEPN=$(LIB1TARGET)

DEF1NAME	=$(SHL1TARGET)
DEF1DEPN	=$(LIB1TARGET)
DEF1DES		=TK
DEFLIB1NAME	=tk

RESLIB1IMAGES=$(PRJ)$/source$/awt $(SOLARSRC)/$(RSCDEFIMG)/$(TARGET)/res
RES1FILELIST=$(SRS)$/awt.srs
RESLIB1NAME=$(TARGET)
RESLIB1SRSFILES=$(RES1FILELIST)

# --- Footer -------------------------------------------------------------
.INCLUDE :	target.mk

ALLTAR : $(MISC)/tk.component

$(MISC)/tk.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        tk.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt tk.component
