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

PRJNAME=slideshow
PRJINC=$(PRJ)$/source
TARGET=tests
TARGETTYPE=GUI

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE : $(PRJ)$/util$/makefile.pmk

CFLAGSCXX += $(CPPUNIT_CFLAGS)

# --- Common ----------------------------------------------------------

# BEGIN target1 -------------------------------------------------------
SHL1OBJS=  \
    $(SLO)$/views.obj	  \
    $(SLO)$/slidetest.obj \
    $(SLO)$/testshape.obj \
    $(SLO)$/testview.obj

SHL1TARGET= tests
SHL1STDLIBS= 	$(SALLIB)		 \
                $(BASEGFXLIB)	 \
                $(CPPUHELPERLIB) \
                $(CPPULIB)		 \
                $(CPPUNITLIB)	 \
                $(UNOTOOLSLIB)	 \
                $(VCLLIB)

.IF "$(OS)"=="WNT"
    SHL1STDLIBS+=$(LIBPRE) islideshowtest.lib
.ELSE
    SHL1STDLIBS+=-lslideshowtest$(DLLPOSTFIX)
.ENDIF

SHL1IMPLIB= i$(SHL1TARGET)

DEF1NAME    =$(SHL1TARGET)
SHL1VERSIONMAP = export.map
SHL1RPATH = NONE

# END target1 ----------------------------------------------------------

# BEGIN target2 --------------------------------------------------------
APP2OBJS=  \
    $(SLO)$/demoshow.obj

APP2TARGET= demoshow
APP2STDLIBS=$(TOOLSLIB) 		\
            $(COMPHELPERLIB)	\
            $(CPPCANVASLIB)		\
            $(CPPULIB)			\
            $(CPPUHELPERLIB)	\
            $(UCBHELPERLIB)		\
            $(SALLIB)			\
            $(VCLLIB)			\
            $(BASEGFXLIB)

.IF "$(GUI)"!="UNX"
APP2DEF=	$(MISC)$/$(TARGET).def
.ENDIF
# END target2 ----------------------------------------------------------------

#------------------------------- All object files ----------------------------

# do this here, so we get right dependencies
SLOFILES=$(SHL1OBJS)

# --- Targets ------------------------------------------------------

.INCLUDE : target.mk
.INCLUDE : _cppunit.mk
