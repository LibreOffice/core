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
# $Revision: 1.21 $
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

PRJNAME=	ucbhelper
TARGET=		provider
AUTOSEG=	TRUE

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk
.INCLUDE :	$(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

.IF "$(header)" == ""

SLOFILES=\
        $(SLO)$/contentidentifier.obj  \
        $(SLO)$/providerhelper.obj     \
        $(SLO)$/contenthelper.obj      \
        $(SLO)$/contentinfo.obj        \
        $(SLO)$/propertyvalueset.obj   \
        $(SLO)$/registerucb.obj        \
        $(SLO)$/resultsetmetadata.obj  \
        $(SLO)$/resultset.obj          \
        $(SLO)$/resultsethelper.obj    \
        $(SLO)$/commandenvironmentproxy.obj     \
        $(SLO)$/interactionrequest.obj          \
        $(SLO)$/simpleinteractionrequest.obj    \
        $(SLO)$/simpleauthenticationrequest.obj \
        $(SLO)$/simplenameclashresolverequest.obj \
        $(SLO)$/simpleioerrorrequest.obj        \
        $(SLO)$/cancelcommandexecution.obj \
        $(SLO)$/handleinteractionrequest.obj \
        $(SLO)$/simplecertificatevalidationrequest.obj

.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

