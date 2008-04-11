#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.5 $
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

#i20156 - new file for xmlsecurity module

PRJ=..$/..$/..$/..$/..$/..

PRJNAME=offapi

TARGET=xsec-sax
PACKAGE=com$/sun$/star$/xml$/crypto$/sax

# --- Settings -----------------------------------------------------
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# ------------------------------------------------------------------------

IDLFILES=\
    XSecurityController.idl       \
    XElementStackKeeper.idl       \
    XSAXEventKeeper.idl        \
    XSecuritySAXEventKeeper.idl        \
    XSAXEventKeeperStatusChangeBroadcaster.idl        \
    XSAXEventKeeperStatusChangeListener.idl        \
    XKeyCollector.idl        \
    XReferenceCollector.idl        \
    XBlockerMonitor.idl        \
    XMissionTaker.idl        \
    XEncryptionResultBroadcaster.idl        \
    XEncryptionResultListener.idl        \
    XDecryptionResultBroadcaster.idl        \
    XDecryptionResultListener.idl        \
    XSignatureCreationResultBroadcaster.idl        \
    XSignatureCreationResultListener.idl        \
    XSignatureVerifyResultBroadcaster.idl        \
    XSignatureVerifyResultListener.idl        \
    XReferenceResolvedBroadcaster.idl        \
    XReferenceResolvedListener.idl        \
    SignatureCreator.idl        \
    SignatureVerifier.idl        \
    Decryptor.idl        \
    Encryptor.idl        \
    SAXEventKeeper.idl

# ------------------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk
