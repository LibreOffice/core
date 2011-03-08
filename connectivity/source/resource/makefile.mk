#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
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
PRJINC=..
PRJNAME=connectivity
# common resources in connectivity
TARGET=cnr
# resources used for logging in the various SDBC drivers
TARGET2=sdbcl
# resources used for ::com::sun::star::sdb::ErrorCondition messages
TARGET3=sdberr

ENABLE_EXCEPTIONS=TRUE

# --- Settings ----------------------------------
.IF "$(DBGUTIL_OJ)"!=""
ENVCFLAGS+=/FR$(SLO)$/
.ENDIF

.INCLUDE : settings.mk
.INCLUDE :  $(PRJ)$/dbtools.pmk
.INCLUDE :  $(PRJ)$/version.mk

CDEFS+=-DCONN_SHARED_RESOURCE_FILE=$(TARGET)

# --- Files -------------------------------------

EXCEPTIONSFILES=\
    $(SLO)$/sharedresources.obj

SLOFILES=\
        $(EXCEPTIONSFILES)

# ...............................................

SRS1NAME=conn_shared_res
SRC1FILES= \
    $(SRS1NAME).src

# ...............................................

SRS2NAME=conn_log_res
SRC2FILES= \
    $(SRS2NAME).src

# ...............................................

SRS3NAME=conn_error_message
SRC3FILES= \
    $(SRS3NAME).src

# === .res file ==========================================================

# ...............................................

RES1FILELIST=\
    $(SRS)$/$(SRS1NAME).srs \

RESLIB1NAME=$(TARGET)
RESLIB1SRSFILES=$(RES1FILELIST)

# ...............................................

RES2FILELIST=\
    $(SRS)$/$(SRS2NAME).srs \

RESLIB2NAME=$(TARGET2)
RESLIB2SRSFILES=$(RES2FILELIST)

# ...............................................

RES3FILELIST=\
    $(SRS)$/$(SRS3NAME).srs \

RESLIB3NAME=$(TARGET3)
RESLIB3SRSFILES=$(RES3FILELIST)

# --- Targets ----------------------------------

.INCLUDE : target.mk


