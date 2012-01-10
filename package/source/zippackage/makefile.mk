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
PRJNAME=package
TARGET=zippackage
AUTOSEG=true

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE : settings.mk

.IF "$(L10N_framework)"==""

# --- Files --------------------------------------------------------
# the following flag un-inlines function calls and disables optimisations
#CFLAGS+=/Ob0 /Od

SLOFILES= \
        $(SLO)$/ZipPackage.obj			\
        $(SLO)$/ZipPackageBuffer.obj	\
        $(SLO)$/ZipPackageEntry.obj		\
        $(SLO)$/ZipPackageFolder.obj	\
        $(SLO)$/ZipPackageFolderEnumeration.obj	\
        $(SLO)$/ZipPackageSink.obj		\
        $(SLO)$/ZipPackageStream.obj	\
        $(SLO)$/wrapstreamforshare.obj	\
        $(SLO)$/zipfileaccess.obj

#		$(SLO)$/InteractionRequest.obj  \
#		$(SLO)$/InteractionContinuation.obj

.ENDIF # L10N_framework

# --- Targets ------------------------------------------------------

.INCLUDE : target.mk
