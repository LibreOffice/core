#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#
PRJ=..$/..

PRJNAME=io
TARGET=teststm
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
