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



PRJ=..

PRJNAME=np_sdk
TARGET=npsdk

EXTERNAL_WARNINGS_NOT_ERRORS := TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk
.IF "$(L10N_framework)"==""
JDKINCS=

# --- Files --------------------------------------------------------

.IF "$(GUI)" == "WNT" || "$(GUI)" == "OS2"
MOZFILES = $(SLO)$/npwin.obj
.ELSE
MOZFILES = $(SLO)$/npunix.obj
.ENDIF

.IF "$(GUIBASE)" == "aqua"
CDEFS+=-DNO_X11
.ENDIF

ALL: $(MOZFILES)

# --- Targets ------------------------------------------------------
.ENDIF 		# L10N_framework

.INCLUDE :	target.mk

