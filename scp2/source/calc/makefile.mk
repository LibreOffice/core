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
TARGET=calc
TARGETTYPE=CUI

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

.IF "$(ENABLE_COINMP)" == "YES"
SCPDEFS+=-DENABLE_COINMP
.IF "$(SYSTEM_COINMP)" == "NO"
.IF "$(GUI)"!="WNT"
# For the non-Windows platforms all libraries related to CoinMP have to be
# added explicitly.
SCPDEFS+=-DADD_ALL_COINMP_LIBRARIES
.ELSE
SCPDEFS+=-DADD_MAIN_COINMP_LIBRARY
.ENDIF
.ENDIF
.ENDIF

SCP_PRODUCT_TYPE=osl
PARFILES= \
        module_calc.par              \
        file_calc.par 

.IF "$(GUI)"=="WNT"
PARFILES += \
        registryitem_calc.par        \
        folderitem_calc.par
.ENDIF

ULFFILES= \
        module_calc.ulf              \
        registryitem_calc.ulf        \
        folderitem_calc.ulf 

# --- File ---------------------------------------------------------
.INCLUDE :  target.mk
