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



PRJ=../../..
PRJINC=$(PRJ)/source
PRJNAME=dbaccess
TARGET=uimisc

ENABLE_EXCEPTIONS=TRUE

# --- Settings ----------------------------------

.INCLUDE : settings.mk
.INCLUDE : $(PRJ)/util/makefile.pmk

# --- Files -------------------------------------

# ... resource files ............................

SRS1NAME=$(TARGET)
SRC1FILES =    \
        dbumiscres.src    \
        WizardPages.src

# ... object files ............................

SLOFILES=    \
        $(SLO)/asyncmodaldialog.obj         \
        $(SLO)/imageprovider.obj            \
        $(SLO)/singledoccontroller.obj      \
        $(SLO)/dbsubcomponentcontroller.obj \
        $(SLO)/datasourceconnector.obj      \
        $(SLO)/linkeddocuments.obj          \
        $(SLO)/indexcollection.obj          \
        $(SLO)/UITools.obj                  \
        $(SLO)/WCPage.obj                   \
        $(SLO)/WCopyTable.obj               \
        $(SLO)/WTypeSelect.obj              \
        $(SLO)/TokenWriter.obj              \
        $(SLO)/HtmlReader.obj               \
        $(SLO)/RtfReader.obj                \
        $(SLO)/propertysetitem.obj          \
        $(SLO)/databaseobjectview.obj       \
        $(SLO)/DExport.obj                  \
        $(SLO)/uiservices.obj               \
        $(SLO)/RowSetDrop.obj               \
        $(SLO)/TableCopyHelper.obj          \
        $(SLO)/moduledbu.obj                \
        $(SLO)/WColumnSelect.obj            \
        $(SLO)/WExtendPages.obj             \
        $(SLO)/WNameMatch.obj               \
        $(SLO)/ToolBoxHelper.obj            \
        $(SLO)/stringlistitem.obj           \
        $(SLO)/charsets.obj                 \
        $(SLO)/defaultobjectnamecheck.obj   \
        $(SLO)/dsmeta.obj                   \
        $(SLO)/controllerframe.obj          \
        $(SLO)/propertystorage.obj          \
        $(SLO)/dbaundomanager.obj           \

# --- Targets ----------------------------------

.INCLUDE : target.mk

