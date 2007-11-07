#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.20 $
#
#   last change: $Author: rt $ $Date: 2007-11-07 10:08:44 $
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
        $(SLO)$/configureucb.obj       \
        $(SLO)$/contentidentifier.obj  \
        $(SLO)$/providerhelper.obj     \
        $(SLO)$/contenthelper.obj      \
        $(SLO)$/contentinfo.obj        \
        $(SLO)$/propertyvalueset.obj   \
        $(SLO)$/registerucb.obj        \
        $(SLO)$/resultsetmetadata.obj  \
        $(SLO)$/resultset.obj          \
        $(SLO)$/resultsethelper.obj    \
        $(SLO)$/provconf.obj	       \
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

