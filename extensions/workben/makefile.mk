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

PRJ=..

PRJNAME=extensions
TARGET=workben
LIBTARGET=NO

TESTAPP=testpgp

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
.ENDIF # phytontest

#
# python auto test
#
.IF "$(TESTAPP)" == "phytonautotest"
OBJFILES=	$(OBJ)$/pythonautotest.obj
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

