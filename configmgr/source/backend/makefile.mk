#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.14 $
#
#   last change: $Author: rt $ $Date: 2005-09-08 03:32:45 $
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

PRJINC=$(PRJ)$/source
PRJNAME=configmgr
TARGET=backend
ENABLE_EXCEPTIONS=TRUE

# --- Settings ---

.INCLUDE : settings.mk

# --- Files ---


SLOFILES=\
    $(SLO)$/mergedcomponentdata.obj \
    $(SLO)$/componentdatahelper.obj \
    $(SLO)$/schemabuilder.obj \
    $(SLO)$/layermerge.obj \
    $(SLO)$/updatesvc.obj \
    $(SLO)$/layerupdatehandler.obj \
    $(SLO)$/basicupdatemerger.obj \
    $(SLO)$/layerupdatemerger.obj \
    $(SLO)$/layerupdatebuilder.obj \
    $(SLO)$/layerupdate.obj \
    $(SLO)$/updatedata.obj \
    $(SLO)$/updatedispatch.obj \
    $(SLO)$/singlebackendadapter.obj \
    $(SLO)$/backendaccess.obj \
    $(SLO)$/backendfactory.obj \
    $(SLO)$/importsvc.obj \
    $(SLO)$/basicimporthandler.obj \
    $(SLO)$/importmergehandler.obj \
    $(SLO)$/layerdefaultremover.obj \
    $(SLO)$/emptylayer.obj \
    $(SLO)$/binarywriter.obj \
    $(SLO)$/binaryreader.obj \
    $(SLO)$/binarywritehandler.obj \
    $(SLO)$/binaryreadhandler.obj	\
    $(SLO)$/binarycache.obj	\
    $(SLO)$/backendnotifier.obj     \
    $(SLO)$/backendstratalistener.obj		\
    $(SLO)$/multistratumbackend.obj	\
    

# --- Targets ---

.INCLUDE : target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk

