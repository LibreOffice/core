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
TARGET=gnome
TARGETTYPE=CUI

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

SCP_PRODUCT_TYPE=osl

.IF "$(ENABLE_GCONF)"!="" || "$(ENABLE_GNOMEVFS)"!="" || "$(ENABLE_GIO)"!=""

.IF "$(ENABLE_GCONF)" != ""
SCPDEFS+=-DENABLE_GCONF
.ENDIF

.IF "$(ENABLE_LOCKDOWN)" == "YES"
SCPDEFS+=-DENABLE_LOCKDOWN
.ENDIF

.IF "$(ENABLE_GNOMEVFS)" != ""
SCPDEFS+=-DENABLE_GNOMEVFS
.ENDIF

.IF "$(ENABLE_GIO)" != ""
SCPDEFS+=-DENABLE_GIO
.ENDIF

PARFILES =                   \
        module_gnome.par     \
        file_gnome.par

ULFFILES= \
        module_gnome.ulf

.ENDIF

# --- File ---------------------------------------------------------

.INCLUDE :  target.mk
