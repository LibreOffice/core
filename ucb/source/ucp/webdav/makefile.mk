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

# UCP Version - Increase, if UCP libraray becomes incompatible.
UCP_VERSION=1

# Name for the UCP. Will become part of the library name (See below).
UCP_NAME=dav

# Relative path to project root.
PRJ = ..$/..$/..

# Project Name.
PRJNAME=ucb

TARGET=ucp$(UCP_NAME)

ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE
NO_BSYMBOLIC=TRUE

# --- Settings ---------------------------------------------------------

.INCLUDE: settings.mk
.IF "$(L10N_framework)"==""

.IF "$(DISABLE_NEON)" == "TRUE"

@all:
    @echo "neon disabled...."

.ELSE

NEONINCDIR=external$/neon

.IF "$(SYSTEM_NEON)" != "YES"
.INCLUDE: $(SOLARINCDIR)$/$(NEONINCDIR)$/version.mk
.ENDIF

CFLAGS+= -DNEON_VERSION=0x$(NEON_VERSION)

.IF "$(SYSTEM_NEON)" == "YES"
CFLAGS+= $(NEON_CFLAGS)
.ELSE
CFLAGS+= -I$(SOLARINCDIR)$/$(NEONINCDIR)
.ENDIF

.IF "$(SYSTEM_LIBXML)" == "YES"
CFLAGS+= $(LIBXML_CFLAGS)
.ELSE
LIBXMLINCDIR=external$/libxml
CFLAGS+= -I$(SOLARINCDIR)$/$(LIBXMLINCDIR)
.ENDIF

.IF "$(SYSTEM_OPENSSL)" == "YES"
CFLAGS+= $(OPENSSL_CFLAGS)
.ENDIF

# --- General -----------------------------------------------------

SLOFILES=\
    $(SLO)$/webdavservices.obj \
    $(SLO)$/webdavprovider.obj \
    $(SLO)$/webdavcontent.obj \
    $(SLO)$/webdavcontentcaps.obj \
    $(SLO)$/webdavresultset.obj \
    $(SLO)$/webdavdatasupplier.obj \
    $(SLO)$/ContentProperties.obj \
    $(SLO)$/DAVProperties.obj \
    $(SLO)$/DAVSessionFactory.obj \
    $(SLO)$/DAVResourceAccess.obj \
    $(SLO)$/NeonUri.obj \
    $(SLO)$/NeonInputStream.obj \
    $(SLO)$/NeonPropFindRequest.obj \
    $(SLO)$/NeonHeadRequest.obj \
    $(SLO)$/NeonSession.obj \
    $(SLO)$/NeonLockStore.obj \
    $(SLO)$/DateTimeHelper.obj \
    $(SLO)$/LinkSequence.obj \
    $(SLO)$/LockSequence.obj \
    $(SLO)$/LockEntrySequence.obj \
    $(SLO)$/UCBDeadPropertyValue.obj

LIB1TARGET=$(SLB)$/_$(TARGET).lib
LIB1OBJFILES=$(SLOFILES)

# --- Shared-Library ---------------------------------------------------

SHL1TARGET=$(TARGET)$(UCP_VERSION)
SHL1IMPLIB=i$(TARGET)

SHL1VERSIONMAP=$(SOLARENV)/src/component.map

SHL1STDLIBS=\
        $(CPPUHELPERLIB) \
        $(CPPULIB)       \
        $(SALLIB)        \
        $(SALHELPERLIB)  \
        $(UCBHELPERLIB)  \
        $(COMPHELPERLIB) \
        $(NEON3RDLIB)    \
        $(LIBXML2LIB)

.IF "$(GUI)"=="WNT"
SHL1STDLIBS+= $(WSOCK32LIB)
.IF "$(WINDOWS_VISTA_PSDK)" != ""
SHL1STDLIBS+= $(WS2_32LIB)
.ENDIF
SHL1STDLIBS+= $(OPENSSLLIB)
.ELSE # WNT
.IF "$(OS)"=="SOLARIS"
SHL1STDLIBS+= -lnsl -lsocket -ldl
.ENDIF # SOLARIS
.IF "$(OS)"=="OS2"
SHL1STDLIBS+= pthread.lib libz.lib
.ENDIF # OS2
.IF "$(SYSTEM_OPENSSL)"=="YES"
SHL1STDLIBS+= $(OPENSSLLIB)
.ELSE
SHL1STDLIBS+= $(OPENSSLLIBST)
.ENDIF
.ENDIF # WNT

SHL1DEF=$(MISC)$/$(SHL1TARGET).def
SHL1LIBS=$(LIB1TARGET)

# --- Def-File ---------------------------------------------------------

DEF1NAME=$(SHL1TARGET)

.ENDIF #"$(DISABLE_NEON)" == "TRUE"

.ENDIF # L10N_framework
# --- Targets ----------------------------------------------------------

.INCLUDE: target.mk

ALLTAR : $(MISC)/ucpdav1.component

$(MISC)/ucpdav1.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        ucpdav1.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt ucpdav1.component
