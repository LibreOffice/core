#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.10 $
#
#   last change: $Author: rt $ $Date: 2007-11-07 10:06:47 $
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

.IF "$(CRYPTO_ENGINE)" == "mscrypto"
CDEFS += -DXMLSEC_CRYPTO_MSCRYPTO -DXMLSEC_NO_XSLT
.ELSE
CDEFS += -DXMLSEC_CRYPTO_NSS -DXMLSEC_NO_XSLT
.IF "$(WITH_MOZILLA)" == "NO"
@all:
    @echo "No mozilla -> no nss -> no libxmlsec -> no xmlsecurity.."
.ENDIF
.ENDIF

# --- Files --------------------------------------------------------
SLOFILES = \
    $(SLO)$/baseencoding.obj \
    $(SLO)$/biginteger.obj \
    $(SLO)$/certvalidity.obj \
    $(SLO)$/saxhelper.obj \
    $(SLO)$/xmldocumentwrapper_xmlsecimpl.obj \
    $(SLO)$/xmlelementwrapper_xmlsecimpl.obj \
    $(SLO)$/certificateextension_xmlsecimpl.obj \
    $(SLO)$/xmlstreamio.obj \
    $(SLO)$/errorcallback.obj \
    $(SLO)$/xsec_xmlsec.obj 
    
# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
