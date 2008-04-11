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
# $Revision: 1.18 $
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

PRJNAME=javaunohelper
TARGET=juh
USE_DEFFILE=TRUE
ENABLE_EXCEPTIONS=TRUE
LIBTARGET=NO

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

.IF "$(SOLAR_JAVA)"==""
nojava:
    @echo "Not building javaunohelper because Java is disabled"
.ENDIF

# ------------------------------------------------------------------

UNOUCRDEP=$(SOLARBINDIR)$/udkapi.rdb
UNOUCRRDB=$(SOLARBINDIR)$/udkapi.rdb

UNOUCROUT=$(OUT)$/inc$/comprehensive
INCPRE+=$(OUT)$/inc$/comprehensive
NO_OFFUH=TRUE
CPPUMAKERFLAGS+=-C

UNOTYPES= \
        com.sun.star.beans.NamedValue \
        com.sun.star.container.XHierarchicalNameAccess		\
        com.sun.star.loader.XImplementationLoader		\
        com.sun.star.registry.XRegistryKey			\
        com.sun.star.registry.XSimpleRegistry			\
        com.sun.star.beans.XPropertySet				\
         com.sun.star.lang.DisposedException			\
        com.sun.star.lang.IllegalArgumentException		\
        com.sun.star.lang.XTypeProvider				\
        com.sun.star.lang.XServiceInfo				\
        com.sun.star.lang.XMultiServiceFactory			\
        com.sun.star.lang.XMultiComponentFactory		\
        com.sun.star.lang.XSingleServiceFactory			\
        com.sun.star.lang.XSingleComponentFactory   		\
        com.sun.star.uno.TypeClass				\
        com.sun.star.uno.XWeak					\
        com.sun.star.uno.XAggregation				\
            com.sun.star.uno.XComponentContext          		\
        com.sun.star.lang.XInitialization           		\
        com.sun.star.lang.XComponent

SLOFILES= \
        $(SLO)$/javaunohelper.obj				\
        $(SLO)$/bootstrap.obj					\
        $(SLO)$/preload.obj \
        $(SLO)$/vm.obj

# ------------------------------------------------------------------

LIB1TARGET=$(SLB)$/$(SHL1TARGET).lib
LIB1OBJFILES=\
        $(SLO)$/javaunohelper.obj				\
        $(SLO)$/bootstrap.obj \
        $(SLO)$/vm.obj

SHL1TARGET=juhx

SHL1STDLIBS= \
        $(JVMACCESSLIB)		\
        $(SALHELPERLIB)		\
        $(SALLIB)		\
        $(CPPULIB)		\
        $(CPPUHELPERLIB)

SHL1VERSIONMAP = javaunohelper.map

SHL1DEPN=
SHL1IMPLIB=i$(SHL1TARGET)
SHL1LIBS=$(LIB1TARGET)
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
SHL1RPATH=URELIB

DEF1NAME=$(SHL1TARGET)

# ------------------------------------------------------------------

LIB2TARGET=$(SLB)$/$(SHL2TARGET).lib
LIB2OBJFILES=\
        $(SLO)$/preload.obj

SHL2TARGET=juh

SHL2STDLIBS= \
        $(SALLIB)

SHL2VERSIONMAP = javaunohelper.map

SHL2DEPN=
SHL2IMPLIB=i$(SHL2TARGET)
SHL2LIBS=$(LIB2TARGET)
SHL2DEF=$(MISC)$/$(SHL2TARGET).def
SHL2RPATH=URELIB

DEF2NAME=$(SHL2TARGET)

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

