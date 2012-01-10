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

PRJNAME=automation
TARGET=server

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

SLOFILES = \
    $(SLO)$/XMLParser.obj \
    $(SLO)$/recorder.obj \
    $(SLO)$/svcommstream.obj \
    $(SLO)$/cmdbasestream.obj \
    $(SLO)$/scmdstrm.obj \
    $(SLO)$/statemnt.obj \
    $(SLO)$/sta_list.obj \
    $(SLO)$/editwin.obj \
    $(SLO)$/server.obj \
    $(SLO)$/retstrm.obj \
    $(SLO)$/profiler.obj


OBJFILES = \
    $(OBJ)$/cmdbasestream.obj \
    $(OBJ)$/svcommstream.obj

EXCEPTIONSFILES = \
    $(SLO)$/statemnt.obj \
    $(SLO)$/server.obj \
    $(SLO)$/XMLParser.obj

NOOPTFILES = $(SLO)$/statemnt.obj

.IF "$(OS)"=="SOLARIS"
SLOFILES +=   $(SLO)$/prof_usl.obj 
.ELSE
SLOFILES +=   $(SLO)$/prof_nul.obj 
.ENDIF

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk
