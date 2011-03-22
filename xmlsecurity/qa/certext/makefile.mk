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
#***********************************************************************/
.IF "$(OOO_SUBSEQUENT_TESTS)" == ""
nothing .PHONY:
.ELSE 

PRJ = ../..
PRJNAME = xmlsecurity
TARGET = qa_certext

ENABLE_EXCEPTIONS = TRUE

.INCLUDE: settings.mk
.INCLUDE :	$(PRJ)$/util$/target.pmk

CFLAGSCXX += $(CPPUNIT_CFLAGS)

SHL1IMPLIB = i$(SHL1TARGET)
SHL1OBJS = $(SLOFILES)
SHL1RPATH = NONE
SHL1STDLIBS = $(CPPUNITLIB)     \
              $(SALLIB)         \
              $(NEON3RDLIB)     \
              $(CPPULIB)        \
              $(XMLOFFLIB)      \
              $(CPPUHELPERLIB)	\
              $(SVLLIB)			\
              $(TOOLSLIB)	    \
              $(COMPHELPERLIB) \
              $(TESTLIB)

SHL1TARGET = qa_CertExt
SHL1VERSIONMAP = $(PRJ)/qa/certext/export.map
DEF1NAME = $(SHL1TARGET)

SLOFILES = $(SLO)/SanCertExt.obj

.INCLUDE: target.mk
.INCLUDE: installationtest.mk

ALLTAR : cpptest

cpptest : $(SHL1TARGETN)

CPPTEST_LIBRARY = $(SHL1TARGETN)

.END