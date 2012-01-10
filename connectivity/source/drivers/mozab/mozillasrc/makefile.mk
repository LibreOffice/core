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


CALL_CDECL=TRUE

#mozilla specific stuff.
MOZ_LIB=$(SOLARVERSION)$/$(INPATH)$/lib$(UPDMINOREXT)
MOZ_INC=$(SOLARVERSION)$/$(INPATH)$/inc$(UPDMINOREXT)$/mozilla
#End of mozilla specific stuff.

PRJ=..$/..$/..$/..
PRJINC=..$/..$/..
PRJNAME=connectivity
TARGET=mozabsrc

# --- Settings ----------------------------------

USE_DEFFILE=TRUE
ENABLE_EXCEPTIONS=TRUE
VISIBILITY_HIDDEN=TRUE

.INCLUDE : settings.mk

.IF  ("$(SYSTEM_MOZILLA)" == "YES" && "$(WITH_MOZILLA)" == "YES") || "$(WITH_MOZILLA)" == "NO" || "$(OS)" == "OS2"
dummy:
    @echo "		Not building the mozillasrc stuff in OpenOffice.org build"
    @echo "		dependency to Mozilla developer snapshots not feasable at the moment"
    @echo "         see http://bugzilla.mozilla.org/show_bug.cgi?id=135137"
.ELSE

.INCLUDE :  $(PRJ)$/version.mk

.INCLUDE : ../makefile_mozab.mk

INCPRE += -I../bootstrap

# --- Files -------------------------------------

SLOFILES = \
    $(SLO)$/MQueryHelper.obj			    \
    $(SLO)$/MDatabaseMetaDataHelper.obj		\
    $(SLO)$/MQuery.obj			            \
    $(SLO)$/MTypeConverter.obj              \
    $(SLO)$/MNameMapper.obj					\
    $(SLO)$/MNSMozabProxy.obj	            \
    $(SLO)$/MNSTerminateListener.obj        \
    $(SLO)$/MLdapAttributeMap.obj           \

.ENDIF

# --- Targets ----------------------------------

.INCLUDE : target.mk

