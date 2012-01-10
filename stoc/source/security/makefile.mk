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

PRJNAME=stoc
TARGET = security
ENABLE_EXCEPTIONS=TRUE
BOOTSTRAP_SERVICE=TRUE
UNOUCROUT=$(OUT)$/inc$/bootstrap

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# ------------------------------------------------------------------

SLOFILES= \
        $(SLO)$/permissions.obj			\
        $(SLO)$/access_controller.obj		\
        $(SLO)$/file_policy.obj

.IF "$(debug)" != ""

# some diagnose
.IF "$(diag)" == "full"
CFLAGS += -D__DIAGNOSE -D__CACHE_DIAGNOSE
.ELIF "$(diag)" == "cache"
CFLAGS += -D__CACHE_DIAGNOSE
.ELIF "$(diag)" != ""
CFLAGS += -D__DIAGNOSE
.ENDIF

.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

