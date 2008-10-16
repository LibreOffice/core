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
# $Revision: 1.10.2.3 $
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

PRJ=..$/..$/..
PRJINC=$(PRJ)$/source
PRJNAME=dbaccess
TARGET=dataaccess

ENABLE_EXCEPTIONS=TRUE

# --- Settings ----------------------------------

.INCLUDE : settings.mk

# --- Files -------------------------------------

SLOFILES=	\
        $(SLO)$/SharedConnection.obj	\
        $(SLO)$/ContentHelper.obj		\
        $(SLO)$/bookmarkcontainer.obj	\
        $(SLO)$/definitioncontainer.obj	\
        $(SLO)$/commanddefinition.obj	\
        $(SLO)$/documentcontainer.obj	\
        $(SLO)$/commandcontainer.obj	\
        $(SLO)$/documentdefinition.obj	\
        $(SLO)$/ComponentDefinition.obj	\
        $(SLO)$/databasecontext.obj		\
        $(SLO)$/connection.obj			\
        $(SLO)$/datasource.obj			\
        $(SLO)$/intercept.obj			\
        $(SLO)$/myucp_datasupplier.obj	\
        $(SLO)$/myucp_resultset.obj		\
        $(SLO)$/databasedocument.obj	\
        $(SLO)$/dataaccessdescriptor.obj\
        $(SLO)$/ModelImpl.obj           \
        $(SLO)$/documentevents.obj      \
        $(SLO)$/documenteventexecutor.obj \
        $(SLO)$/documenteventnotifier.obj

# --- Targets ----------------------------------

.INCLUDE : target.mk

