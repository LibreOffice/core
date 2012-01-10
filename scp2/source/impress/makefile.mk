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
TARGET=impress
TARGETTYPE=CUI

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

SCP_PRODUCT_TYPE=osl

PARFILES= \
        module_impress.par              \
        file_impress.par 

.IF "$(GUI)"=="WNT"
PARFILES += \
        registryitem_impress.par        \
        folderitem_impress.par
.ENDIF

ULFFILES= \
        module_impress.ulf              \
        registryitem_impress.ulf        \
        folderitem_impress.ulf

.IF "$(ENABLE_OPENGL)" == "TRUE"
PARFILES+=\
        module_ogltrans.par
ULFFILES+=\
    module_ogltrans.ulf
.ENDIF

# --- File ---------------------------------------------------------
.INCLUDE :  target.mk
