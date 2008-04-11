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
# $Revision: 1.45 $
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
PRJNAME=comphelper
TARGET=misc

ENABLE_EXCEPTIONS=TRUE

# --- Settings ----------------------------------

.INCLUDE : settings.mk
.INCLUDE : $(PRJ)$/util$/makefile.pmk

# --- Files -------------------------------------

SLOFILES=	\
            $(SLO)$/weak.obj\
            $(SLO)$/asyncnotification.obj \
            $(SLO)$/listenernotification.obj \
            $(SLO)$/evtlistenerhlp.obj \
            $(SLO)$/accessiblecomponenthelper.obj \
            $(SLO)$/accessiblecontexthelper.obj \
            $(SLO)$/accessiblekeybindinghelper.obj \
            $(SLO)$/accessibleselectionhelper.obj \
            $(SLO)$/accessibletexthelper.obj \
            $(SLO)$/accimplaccess.obj \
            $(SLO)$/interaction.obj \
            $(SLO)$/instancelocker.obj \
            $(SLO)$/types.obj \
            $(SLO)$/numbers.obj \
            $(SLO)$/sequence.obj \
            $(SLO)$/querydeep.obj \
            $(SLO)$/regpathhelper.obj \
            $(SLO)$/facreg.obj \
            $(SLO)$/accessibleeventbuffer.obj	\
            $(SLO)$/weakeventlistener.obj	\
            $(SLO)$/accessibleeventnotifier.obj	 \
            $(SLO)$/accessiblewrapper.obj	\
            $(SLO)$/proxyaggregation.obj    \
            $(SLO)$/sequenceashashmap.obj	\
            $(SLO)$/storagehelper.obj \
            $(SLO)$/anytostring.obj		\
            $(SLO)$/mediadescriptor.obj \
            $(SLO)$/locale.obj \
            $(SLO)$/configurationhelper.obj \
            $(SLO)$/mimeconfighelper.obj \
            $(SLO)$/servicedecl.obj \
            $(SLO)$/scopeguard.obj \
            $(SLO)$/componentmodule.obj \
            $(SLO)$/componentcontext.obj \
            $(SLO)$/officeresourcebundle.obj \
            $(SLO)$/SelectionMultiplex.obj \
            $(SLO)$/namedvaluecollection.obj \
            $(SLO)$/numberedcollection.obj \
            $(SLO)$/logging.obj \
            $(SLO)$/documentinfo.obj \
            $(SLO)$/string.obj \
            $(SLO)$/legacysingletonfactory.obj

# --- Targets ----------------------------------

.INCLUDE : target.mk
