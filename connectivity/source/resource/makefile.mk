#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



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
.INCLUDE :  $(PRJ)$/dbtools.pmk
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


