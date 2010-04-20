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
# $Revision: 1.6 $
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
PRJ=..$/..

PRJNAME=io
TARGET=teststm
NO_BSYMBOLIC=TRUE
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------
.INCLUDE :  settings.mk
.IF "$(L10N_framework)"==""
# --- Files --------------------------------------------------------
UNOUCRDEP=$(SOLARBINDIR)$/udkapi.rdb
UNOUCRRDB=$(SOLARBINDIR)$/udkapi.rdb

# output directory (one dir for each project)
UNOUCROUT=$(OUT)$/inc

UNOTYPES =	com.sun.star.test.XSimpleTest \
        com.sun.star.beans.XPropertySet \
        com.sun.star.io.UnexpectedEOFException	\
        com.sun.star.io.WrongFormatException	\
        com.sun.star.io.XActiveDataControl	\
        com.sun.star.io.XActiveDataSink	\
        com.sun.star.io.XActiveDataSource	\
        com.sun.star.io.XConnectable	\
        com.sun.star.io.XMarkableStream	\
        com.sun.star.io.XObjectInputStream	\
        com.sun.star.io.XObjectOutputStream	\
        com.sun.star.lang.IllegalArgumentException	\
        com.sun.star.lang.XComponent	\
        com.sun.star.lang.XMultiServiceFactory	\
        com.sun.star.lang.XServiceInfo	\
        com.sun.star.lang.XSingleServiceFactory	\
        com.sun.star.lang.XSingleComponentFactory	\
        com.sun.star.lang.XMultiComponentFactory	\
        com.sun.star.uno.XComponentContext	\
        com.sun.star.lang.XTypeProvider	\
        com.sun.star.registry.XImplementationRegistration	\
        com.sun.star.registry.XRegistryKey	\
        com.sun.star.test.XSimpleTest	\
        com.sun.star.uno.TypeClass	\
        com.sun.star.uno.XAggregation	\
        com.sun.star.uno.XWeak

SLOFILES=	 \
        $(SLO)$/testfactreg.obj \
        $(SLO)$/pipetest.obj \
        $(SLO)$/datatest.obj \
        $(SLO)$/marktest.obj \
        $(SLO)$/pumptest.obj

SHL1TARGET= $(TARGET)

SHL1STDLIBS= \
        $(SALLIB) 	 \
        $(CPPULIB) \
        $(CPPUHELPERLIB)

SHL1LIBS=		$(SLB)$/$(TARGET).lib

SHL1IMPLIB=		i$(TARGET)

SHL1DEPN=		makefile.mk $(SHL1LIBS)
SHL1DEF=		$(MISC)$/$(SHL1TARGET).def

DEF1NAME=		$(SHL1TARGET)
DEF1EXPORTFILE=	exports.dxp


# --- Targets ------------------------------------------------------
.ENDIF 		# L10N_framework

.INCLUDE :	target.mk
