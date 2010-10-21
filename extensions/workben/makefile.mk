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

PRJNAME=extensions
TARGET=workben
LIBTARGET=NO

TESTAPP=testpgp
#TESTAPP=testcomponent
#TESTAPP=pythontest
#TESTAPP=pythonautotest
#TESTAPP=testresource
#TESTAPP=testframecontrol

.IF "$(TESTAPP)" == "testresource" || "$(TESTAPP)" == "testframecontrol"
TARGETTYPE=GUI
.ELSE
TARGETTYPE=CUI
.ENDIF

JVM_TARGET=jvmtest

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

UNOUCROUT=$(OUT)$/inc$/$(PRJNAME)$/$(TARGET)
INCPRE+=$(UNOUCROUT)

# --- Files --------------------------------------------------------

#
# testpgp.
#
.IF "$(TESTAPP)" == "testpgp"

UNOTYPES= \
    com.sun.star.lang.XMultiServiceFactory \
    com.sun.star.io.XInputStream \
    com.sun.star.io.XOutputStream \
    com.sun.star.pgp.RecipientsEvent \
    com.sun.star.pgp.SignatureEvent \
    com.sun.star.pgp.XPGPDecoder \
    com.sun.star.pgp.XPGPDecoderListener \
    com.sun.star.pgp.XPGPEncoder \
    com.sun.star.pgp.XPGPPreferences \
    com.sun.star.uno.TypeClass \
    com.sun.star.uno.XInterface \
    com.sun.star.uno.XWeak

OBJFILES=	$(OBJ)$/testpgp.obj

APP1TARGET=	testpgp
APP1OBJS=	$(OBJFILES)
APP1STDLIBS= \
            $(CPPUHELPERLIB)\
            $(CPPULIB)		\
            $(SALLIB)

.ENDIF # testpgp

#
# std testcomponent
#
.IF "$(TESTAPP)" == "testcomponent"

OBJFILES=	$(OBJ)$/testcomponent.obj

APP2TARGET = testcomponent
APP2OBJS   = $(OBJ)$/testcomponent.obj
APP2STDLIBS = \
              $(TOOLSLIB)	\
              $(SALLIB)

.ENDIF # testcomponent

#
# test python.
#
.IF "$(TESTAPP)" == "phytontest"

OBJFILES=	$(OBJ)$/pythontest.obj

#APP4TARGET=	pythontest
#APP4OBJS=	$(OBJ)$/pythontest.obj 
#APP4STDLIBS=$(TOOLSLIB)
#

.ENDIF # phytontest

#
# python auto test
#
.IF "$(TESTAPP)" == "phytonautotest"

OBJFILES=	$(OBJ)$/pythonautotest.obj

#APP5TARGET= pythonautotest
#APP5OBJS = $(OBJ)$/pythonautotest.obj
#APP5STDLIBS=$(TOOLSLIB)

.ENDIF # phytonautotest

#
# testresource.
#
.IF "$(TESTAPP)" == "testresource"

OBJFILES=	$(OBJ)$/testresource.obj

SRS2NAME =		testresource
SRC2FILES=		testresource.src
RESLIB2SRSFILES= $(SRS)$/testresource.srs
RESLIB2NAME=	testresource

APP2TARGET=	testresource
APP2OBJS=	$(OBJ)$/testresource.obj
APP2STDLIBS=$(TOOLSLIB)		\
            $(VCLLIB)		\
            $(SALLIB)

.ENDIF # testresource

#
# testframecontrol.
#
.IF "$(TESTAPP)" == "testframecontrol"

OBJFILES=	$(OBJ)$/testframecontrol.obj

APP3TARGET=	testframecontrol
APP3OBJS=	$(OBJ)$/testframecontrol.obj
APP3STDLIBS=$(TOOLSLIB)		\
            $(SVTOOLLIB)	\
            $(VCLLIB)

.ENDIF # testframecontrol

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

