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
TARGET = xs_mscrypt

ENABLE_EXCEPTIONS = TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :	$(PRJ)$/util$/target.pmk

.IF "$(CRYPTO_ENGINE)" != "mscrypto"
LIBTARGET=NO
.ENDIF

.IF "$(CRYPTO_ENGINE)" == "mscrypto"

.IF "$(WITH_MOZILLA)" == "NO" || "$(ENABLE_NSS_MODULE)"!="YES"
.IF "$(SYSTEM_MOZILLA)" != "YES"
@all:
    @echo "No mozilla -> no nss -> no libxmlsec -> no xmlsecurity/nss"
.ENDIF
.ENDIF

CDEFS += -DXMLSEC_CRYPTO_MSCRYPTO -DXMLSEC_NO_XSLT

# --- Files --------------------------------------------------------
INCLOCAL = \
    ..

SLOFILES = \
    $(SLO)$/securityenvironment_mscryptimpl.obj \
    $(SLO)$/xmlencryption_mscryptimpl.obj \
    $(SLO)$/xmlsecuritycontext_mscryptimpl.obj \
    $(SLO)$/xmlsignature_mscryptimpl.obj \
    $(SLO)$/x509certificate_mscryptimpl.obj \
    $(SLO)$/seinitializer_mscryptimpl.obj \
    $(SLO)$/xsec_mscrypt.obj  \
    $(SLO)$/sanextension_mscryptimpl.obj

.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

