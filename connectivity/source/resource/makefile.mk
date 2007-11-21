#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.9 $
#
#   last change: $Author: ihi $ $Date: 2007-11-21 15:10:30 $
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

CDEFS+=-DCONN_SHARED_RESOURCE_FILE=$(TARGET)

.INCLUDE : settings.mk
.INCLUDE :  $(PRJ)$/version.mk


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


