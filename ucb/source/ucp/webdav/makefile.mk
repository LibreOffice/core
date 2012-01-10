#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



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

.IF "$(SYSTEM_NEON)" != "YES"

@all:
    @echo "no system neon is used...."

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
