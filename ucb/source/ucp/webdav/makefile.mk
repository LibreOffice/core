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

APRINCDIR=apr
APRUTILINCDIR=apr-util
SERFINCDIR=serf

#
# Extend the list of include paths depending on whether we use locally built
# or system versions of libraries apr, apr-util, serf, libxml
#
# We have to use CFLAGS for this because PRJINC is too inflexible (it adds /inc to everyting.)
#

.IF "$(SYSTEM_APR)" == "YES"
CFLAGS+= $(APR_CFLAGS)
SHL1STDLIBS+=$(APR_LIBS)
.ELSE
CFLAGS+= -I$(SOLARINCDIR)$/$(APRINCDIR)
SHL1STDLIBS+=$(INTERNAL_APR_LIBS)
.INCLUDE : apr_version.mk
CFLAGS+=\
    -DAPR_VERSION=\"$(APR_MAJOR).$(APR_MINOR).$(APR_MICRO)\"
.ENDIF

.IF "$(SYSTEM_APR_UTIL)" == "YES"
CFLAGS+= $(APR_UTIL_CFLAGS)
SHL1STDLIBS+=$(APR_UTIL_LIBS)
.ELSE
CFLAGS+= -I$(SOLARINCDIR)$/$(APRUTILINCDIR)
SHL1STDLIBS+=$(INTERNAL_APR_UTIL_LIBS)
.INCLUDE : aprutil_version.mk
CFLAGS+=\
    -DAPR_UTIL_VERSION=\"$(APR_UTIL_MAJOR).$(APR_UTIL_MINOR).$(APR_UTIL_MICRO)\"
.ENDIF

.IF "$(SYSTEM_SERF)" == "YES"
CFLAGS+= $(SERF_CFLAGS)
SHL1STDLIBS+=$(SERF_LIBS)
.ELSE
CFLAGS+= -I$(SOLARINCDIR)$/$(SERFINCDIR)
SHL1STDLIBS+=$(INTERNAL_SERF_LIBS)
.INCLUDE : serf_version.mk
CFLAGS+=\
    -DSERF_VERSION=\"$(SERF_MAJOR).$(SERF_MINOR).$(SERF_MICRO)\"
.ENDIF

.IF "$(SYSTEM_LIBXML)" == "YES"
CFLAGS+= $(LIBXML_CFLAGS)
.ELSE
LIBXMLINCDIR=external$/libxml
CFLAGS+= -I$(SOLARINCDIR)$/$(LIBXMLINCDIR)
.ENDIF

.IF "$(SYSTEM_OPENSSL)" == "YES"
CFLAGS+= -I$(OPENSSL_CFLAGS)
SHL1STDLIBS+= $(OPENSSLLIB)
.ELSE
SHL1STDLIBS+= $(OPENSSLLIBST)
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
    $(SLO)$/AprEnv.obj \
    $(SLO)$/webdavresponseparser.obj \
    $(SLO)$/SerfUri.obj \
    $(SLO)$/SerfRequestProcessor.obj \
    $(SLO)$/SerfRequestProcessorImpl.obj \
    $(SLO)$/SerfRequestProcessorImplFac.obj \
    $(SLO)$/SerfPropFindReqProcImpl.obj \
    $(SLO)$/SerfPropPatchReqProcImpl.obj \
    $(SLO)$/SerfGetReqProcImpl.obj \
    $(SLO)$/SerfHeadReqProcImpl.obj \
    $(SLO)$/SerfPutReqProcImpl.obj \
    $(SLO)$/SerfPostReqProcImpl.obj \
    $(SLO)$/SerfDeleteReqProcImpl.obj \
    $(SLO)$/SerfMkColReqProcImpl.obj \
    $(SLO)$/SerfCopyReqProcImpl.obj \
    $(SLO)$/SerfMoveReqProcImpl.obj \
    $(SLO)$/SerfSession.obj \
    $(SLO)$/SerfCallbacks.obj \
    $(SLO)$/SerfInputStream.obj \
    $(SLO)$/DateTimeHelper.obj \
    $(SLO)$/UCBDeadPropertyValue.obj

LIB1TARGET=$(SLB)$/_$(TARGET).lib
LIB1OBJFILES=$(SLOFILES)

# --- Shared-Library ---------------------------------------------------

SHL1TARGET=$(TARGET)$(UCP_VERSION)
SHL1IMPLIB=i$(TARGET)

SHL1VERSIONMAP=$(SOLARENV)/src/component.map

SHL1STDLIBS+=\
        $(CPPUHELPERLIB) \
        $(CPPULIB)       \
        $(SALLIB)        \
        $(SALHELPERLIB)  \
        $(UCBHELPERLIB)  \
        $(COMPHELPERLIB) \
        $(LIBXML2LIB)

.IF "$(GUI)"=="WNT"
SHL1STDLIBS+= $(WSOCK32LIB)
.IF "$(WINDOWS_VISTA_PSDK)" != ""
SHL1STDLIBS+= $(WS2_32LIB)
.ENDIF
.ELSE # WNT
.IF "$(OS)"=="SOLARIS"
SHL1STDLIBS+= -lnsl -lsocket -ldl
.ENDIF # SOLARIS
.ENDIF # WNT

SHL1DEF=$(MISC)$/$(SHL1TARGET).def
SHL1LIBS=$(LIB1TARGET)

# --- Def-File ---------------------------------------------------------

DEF1NAME=$(SHL1TARGET)

.ENDIF # L10N_framework
# --- Targets ----------------------------------------------------------

.INCLUDE: target.mk

ALLTAR : $(MISC)/ucpdav1.component

$(MISC)/ucpdav1.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        ucpdav1.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt ucpdav1.component
