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

PRJNAME= stoc
TARGET = regtypeprov
ENABLE_EXCEPTIONS=TRUE
BOOTSTRAP_SERVICE=TRUE
UNOUCROUT= $(OUT)$/inc$/bootstrap

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# ------------------------------------------------------------------

SLOFILES=	\
        $(SLO)$/tdprovider.obj	\
        $(SLO)$/td.obj		\
        $(SLO)$/tdef.obj	\
        $(SLO)$/tdenum.obj	\
        $(SLO)$/tdcomp.obj	\
        $(SLO)$/tdconsts.obj	\
        $(SLO)$/tdiface.obj \
        $(SLO)$/tdmodule.obj \
        $(SLO)$/tdprop.obj \
        $(SLO)$/tdservice.obj \
        $(SLO)$/tdsingleton.obj \
        $(SLO)$/rdbtdp_tdenumeration.obj \
        $(SLO)$/functiondescription.obj \
        $(SLO)$/methoddescription.obj \
        $(SLO)$/structtypedescription.obj

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

