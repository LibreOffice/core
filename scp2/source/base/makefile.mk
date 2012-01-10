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

PRJPCH=

PRJNAME=scp2
TARGET=base
TARGETTYPE=CUI

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

.IF "$(ENABLE_REPORTBUILDER)" == "YES"
SCPDEFS += -DENABLE_REPORTBUILDER
.ENDIF

SCP_PRODUCT_TYPE=osl

PARFILES= \
        module_base.par             \
        file_base.par

.IF "$(GUI)"=="WNT"
PARFILES += \
        registryitem_base.par        \
        folderitem_base.par
.ENDIF

ULFFILES= \
        module_base.ulf              \
        registryitem_base.ulf        \
        folderitem_base.ulf

# --- File ---------------------------------------------------------
.INCLUDE :  target.mk
