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
# $Revision: 1.16 $
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

PRJINC=$(PRJ)$/source
PRJNAME=configmgr
TARGET=backend
ENABLE_EXCEPTIONS=TRUE

# --- Settings ---

.INCLUDE : settings.mk
.INCLUDE : $(PRJ)$/makefile.pmk

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

