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

PRJ=..$/..$/..

PRJNAME = xmlsecurity
TARGET = xs_nss

ENABLE_EXCEPTIONS = TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :	$(PRJ)$/util$/target.pmk

.IF "$(SYSTEM_LIBXML)" == "YES"
CFLAGS+=-DSYSTEM_LIBXML $(LIBXML_CFLAGS)
.ENDIF

.IF "$(WITH_MOZILLA)" == "NO" || "$(ENABLE_NSS_MODULE)"!="YES"
.IF "$(SYSTEM_MOZILLA)" != "YES"
@all:
    @echo "No mozilla -> no nss -> no libxmlsec -> no xmlsecurity/nss"
.ENDIF
.ENDIF

.IF "$(SYSTEM_MOZILLA)" != "YES"
MOZ_INC = $(SOLARVERSION)$/$(INPATH)$/inc$(UPDMINOREXT)$/mozilla
NSS_INC = $(MOZ_INC)$/nss
NSPR_INC = $(MOZ_INC)$/nspr
.ELSE
# MOZ_INC already defined from environment
NSS_INC = $(MOZ_NSS_CFLAGS)
NSPR_INC = $(MOZ_INC)$/nspr
.ENDIF

.IF "$(GUI)"=="UNX"
.IF "$(COMNAME)"=="sunpro5"
CFLAGS += -features=tmplife
#This flag is needed to build mozilla 1.7 code
.ENDIF		# "$(COMNAME)"=="sunpro5"
.ENDIF

.IF "$(GUI)" == "WNT"
.IF "$(DBG_LEVEL)" == "0"
INCPRE += \
-I$(MOZ_INC)$/profile \
-I$(MOZ_INC)$/string \
-I$(MOZ_INC)$/embed_base
CFLAGS +=   -GR- -W3 -Gy -MD -UDEBUG
.ELSE
INCPRE += \
-I$(MOZ_INC)$/profile \
-I$(MOZ_INC)$/string \
-I$(MOZ_INC)$/embed_base
CFLAGS += -Zi -GR- -W3 -Gy -MDd -UNDEBUG
.ENDIF
.ENDIF
.IF "$(GUI)" == "UNX"
INCPOST += \
$(MOZ_INC)$/profile \
-I$(MOZ_INC)$/string \
-I$(MOZ_INC)$/embed_base
#.IF "$(OS)" == "LINUX"
#CFLAGS +=   -fPIC -g
#CFLAGSCXX += \
#            -fno-rtti -Wall -Wconversion -Wpointer-arith \
#            -Wbad-function-cast -Wcast-align -Woverloaded-virtual -Wsynth \
#            -Wno-long-long -pthread
#CDEFS     += -DTRACING
#.ELIF "$(OS)" == "NETBSD"
#CFLAGS +=   -fPIC
#CFLAGSCXX += \
#            -fno-rtti -Wall -Wconversion -Wpointer-arith \
#            -Wbad-function-cast -Wcast-align -Woverloaded-virtual -Wsynth \
#            -Wno-long-long
#CDEFS     += -DTRACING
#.ENDIF
.ENDIF

.IF "$(CRYPTO_ENGINE)" == "nss"
CDEFS += -DXMLSEC_CRYPTO_NSS
.ENDIF

CDEFS += -DXMLSEC_NO_XSLT

# --- Files --------------------------------------------------------

SOLARINC += \
 -I$(MOZ_INC) \
-I$(NSPR_INC) \
-I$(PRJ)$/source$/xmlsec

.IF "$(SYSTEM_MOZILLA)" == "YES"
SOLARINC += -DSYSTEM_MOZILLA $(NSS_INC)
.ELSE
SOLARINC += -I$(NSS_INC)
.ENDIF

SLOFILES = \
    $(SLO)$/nssinitializer.obj \
    $(SLO)$/digestcontext.obj \
    $(SLO)$/ciphercontext.obj \
    $(SLO)$/xsec_nss.obj

.IF "$(CRYPTO_ENGINE)" == "nss"
SLOFILES += \
    $(SLO)$/securityenvironment_nssimpl.obj \
    $(SLO)$/seinitializer_nssimpl.obj \
    $(SLO)$/xmlencryption_nssimpl.obj \
    $(SLO)$/xmlsecuritycontext_nssimpl.obj \
    $(SLO)$/xmlsignature_nssimpl.obj \
    $(SLO)$/x509certificate_nssimpl.obj \
    $(SLO)$/sanextension_nssimpl.obj \
    $(SLO)$/secerror.obj

.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
