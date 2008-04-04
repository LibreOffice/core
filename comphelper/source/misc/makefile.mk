#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.44 $
#
#   last change: $Author: kz $ $Date: 2008-04-04 14:23:24 $
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
