#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: rt $ $Date: 2005-09-09 17:29:13 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
#
#*************************************************************************

PRJ=..$/..$/..

PRJNAME = xmlsecurity
TARGET = xs_mscrypt

ENABLE_EXCEPTIONS = TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk
.INCLUDE :	$(PRJ)$/util$/target.pmk

.IF "$(CRYPTO_ENGINE)" != "mscrypto"
LIBTARGET=NO
.ENDIF

.IF "$(CRYPTO_ENGINE)" == "mscrypto"

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
    $(SLO)$/xsec_mscrypt.obj

.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
