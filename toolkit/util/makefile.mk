#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.25 $
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
            $(SLB)$/controls.lib \
            $(SLB)$/helper.lib
SHL1TARGET= tk$(DLLPOSTFIX)
SHL1IMPLIB= itk
SHL1USE_EXPORTS=name

SHL1STDLIBS=\
        $(VOSLIB)           \
        $(VCLLIB)			\
        $(SOTLIB)			\
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

.IF "$(OS)"=="MACOSX" 

# [ed] 6/16/02 Add in X libraries if we're building X

.IF "$(GUIBASE)"=="unx"
SHL1STDLIBS +=\
    -lX11 -lXt -lXmu
.ENDIF

.ELSE
.IF "$(GUI)"=="UNX"
SHL1STDLIBS +=\
    -lX11
.ENDIF
.ENDIF

RESLIB1IMAGES=$(PRJ)$/source$/awt
RES1FILELIST=$(SRS)$/awt.srs
RESLIB1NAME=$(TARGET)
RESLIB1SRSFILES=$(RES1FILELIST)

.IF "$(ENABLE_LAYOUT)" == "TRUE"

TARGET2 = tklayout
LIB2FILES= \
    $(SLB)$/layout_awt.lib \
    $(SLB)$/layoutcore.lib \
    $(SLB)$/vclcompat.lib

LIB2TARGET= $(SLB)$/$(TARGET2).lib
SHL2TARGET= $(TARGET2)$(UPD)$(DLLPOSTFIX)
SHL2IMPLIB= $(TARGET2)

SHL2LIBS= $(LIB2TARGET)

SHL2STDLIBS=  \
    $(UNOTOOLSLIB) \
    $(TKLIB) \
    $(TOOLSLIB) \
    $(COMPHELPERLIB) \
    $(VCLLIB) \
    $(CPPULIB) \
    $(CPPUHELPERLIB) \
    $(SALLIB) \

.ENDIF # ENABLE_LAYOUT == "TRUE"

# --- Footer -------------------------------------------------------------
.INCLUDE :	target.mk
