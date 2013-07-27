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
PRJNAME=dbaccess
TARGET=uicontrols

# --- Settings ----------------------------------

.INCLUDE : settings.mk
.INCLUDE : $(PRJ)$/util$/makefile.pmk

# --- Files -------------------------------------

# ... resource files ............................

SRS1NAME=$(TARGET)
SRC1FILES =	\
        TableGrantCtrl.src	\
        undosqledit.src		\
        tabletree.src		

# ... exception files .........................

EXCEPTIONSFILES=\
        $(SLO)$/RelationControl.obj		\
        $(SLO)$/toolboxcontroller.obj	\
        $(SLO)$/tabletree.obj			\
        $(SLO)$/TableGrantCtrl.obj		\
        $(SLO)$/dbtreelistbox.obj       \
        $(SLO)$/sqledit.obj				\
        $(SLO)$/ColumnControlWindow.obj	\
        $(SLO)$/FieldDescControl.obj    \
        $(SLO)$/opendoccontrols.obj

# ... object files ............................

SLOFILES=	\
        $(EXCEPTIONSFILES)				\
        $(SLO)$/ScrollHelper.obj		\
        $(SLO)$/VertSplitView.obj		\
        $(SLO)$/SqlNameEdit.obj			\
        $(SLO)$/listviewitems.obj		\
        $(SLO)$/undosqledit.obj			\
        $(SLO)$/marktree.obj			\
        $(SLO)$/curledit.obj            \
        $(SLO)$/charsetlistbox.obj

# --- Targets ----------------------------------

.INCLUDE : target.mk

