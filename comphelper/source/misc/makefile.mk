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
# $Revision: 1.48.16.3 $
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

SLOFILES=   \
            $(SLO)$/accessiblecomponenthelper.obj \
            $(SLO)$/accessiblecontexthelper.obj \
            $(SLO)$/accessibleeventbuffer.obj \
            $(SLO)$/accessibleeventnotifier.obj  \
            $(SLO)$/accessiblekeybindinghelper.obj \
            $(SLO)$/accessibleselectionhelper.obj \
            $(SLO)$/accessibletexthelper.obj \
            $(SLO)$/accessiblewrapper.obj \
            $(SLO)$/accimplaccess.obj \
            $(SLO)$/anytostring.obj \
            $(SLO)$/asyncnotification.obj \
            $(SLO)$/componentcontext.obj \
            $(SLO)$/componentmodule.obj \
            $(SLO)$/configurationhelper.obj \
            $(SLO)$/documentinfo.obj \
        $(SLO)$/evtmethodhelper.obj \
            $(SLO)$/evtlistenerhlp.obj \
            $(SLO)$/facreg.obj \
            $(SLO)$/ihwrapnofilter.obj \
            $(SLO)$/instancelocker.obj \
            $(SLO)$/interaction.obj \
            $(SLO)$/legacysingletonfactory.obj \
            $(SLO)$/listenernotification.obj \
            $(SLO)$/locale.obj \
            $(SLO)$/logging.obj \
            $(SLO)$/mediadescriptor.obj \
            $(SLO)$/mimeconfighelper.obj \
            $(SLO)$/namedvaluecollection.obj \
            $(SLO)$/numberedcollection.obj \
            $(SLO)$/numbers.obj \
            $(SLO)$/officeresourcebundle.obj \
            $(SLO)$/proxyaggregation.obj \
            $(SLO)$/querydeep.obj \
            $(SLO)$/regpathhelper.obj \
            $(SLO)$/scopeguard.obj \
            $(SLO)$/SelectionMultiplex.obj \
            $(SLO)$/sequenceashashmap.obj \
            $(SLO)$/sequence.obj \
            $(SLO)$/servicedecl.obj \
            $(SLO)$/sharedmutex.obj \
            $(SLO)$/synchronousdispatch.obj \
            $(SLO)$/storagehelper.obj \
            $(SLO)$/string.obj \
            $(SLO)$/types.obj \
            $(SLO)$/uieventslogger.obj \
            $(SLO)$/weakeventlistener.obj \
            $(SLO)$/weak.obj \

# --- Targets ----------------------------------

.INCLUDE : target.mk

