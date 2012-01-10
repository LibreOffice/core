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



PRJ=..$/..$/..
PRJINC=$(PRJ)$/source
PRJNAME=reportdesign
TARGET=uidlg
IMGLST_SRS=$(SRS)$/$(TARGET).srs
BMP_IN=$(PRJ)$/res

VISIBILITY_HIDDEN=TRUE

# --- Settings ----------------------------------

.INCLUDE : settings.mk
# .INCLUDE : $(PRJ)$/util$/dll.pmk

# --- Files -------------------------------------

# ... resource files ............................

SRS1NAME=$(TARGET)
SRC1FILES =	\
        dlgpage.src	\
        PageNumber.src	\
        DateTime.src	\
        CondFormat.src	\
        Navigator.src	\
        GroupsSorting.src


# ... object files ............................

EXCEPTIONSFILES=	\
        $(SLO)$/dlgpage.obj					\
        $(SLO)$/Condition.obj				\
        $(SLO)$/CondFormat.obj				\
        $(SLO)$/GroupExchange.obj			\
        $(SLO)$/PageNumber.obj				\
        $(SLO)$/DateTime.obj				\
        $(SLO)$/AddField.obj				\
        $(SLO)$/Navigator.obj				\
        $(SLO)$/GroupsSorting.obj           \
        $(SLO)$/Formula.obj

SLOFILES=	\
        $(EXCEPTIONSFILES)					\


# --- Targets ----------------------------------

.INCLUDE : target.mk

