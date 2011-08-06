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

PRJ=..$/..

PRJNAME = xmlsecurity
TARGET = xs_comm

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
    @echo "No mozilla -> no nss -> no libxmlsec -> no xmlsecurity.."
.ENDIF
.ENDIF

.IF "$(CRYPTO_ENGINE)" == "mscrypto"
CDEFS += -DXMLSEC_CRYPTO_MSCRYPTO
.ENDIF

CDEFS += -DXMLSEC_NO_XSLT

# --- Files --------------------------------------------------------
SLOFILES = \
    $(SLO)$/biginteger.obj \
    $(SLO)$/certvalidity.obj \
    $(SLO)$/saxhelper.obj \
    $(SLO)$/xmldocumentwrapper_xmlsecimpl.obj \
    $(SLO)$/xmlelementwrapper_xmlsecimpl.obj \
    $(SLO)$/certificateextension_xmlsecimpl.obj \
    $(SLO)$/xmlstreamio.obj \
    $(SLO)$/errorcallback.obj \
    $(SLO)$/xsec_xmlsec.obj \
        $(SLO)$/diagnose.obj
    
# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
