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



PRJ=..$/..$/..

PRJNAME=sc
TARGET=undo
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  scpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sc.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

CXXFILES = \
        target.cxx \
        refundo.cxx \
        areasave.cxx \
        undobase.cxx \
        undoutil.cxx \
        undocell.cxx \
        undostyl.cxx \
        undoolk.cxx \
        undoblk.cxx \
        undoblk2.cxx \
        undoblk3.cxx \
        undodat.cxx \
        undodraw.cxx \
        undotab.cxx



SLOFILES =  \
        $(SLO)$/target.obj \
        $(SLO)$/refundo.obj \
        $(SLO)$/areasave.obj \
        $(SLO)$/undobase.obj \
        $(SLO)$/undoutil.obj \
        $(SLO)$/undocell.obj \
        $(SLO)$/undostyl.obj \
        $(SLO)$/undoolk.obj \
        $(SLO)$/undoblk.obj \
        $(SLO)$/undoblk2.obj \
        $(SLO)$/undoblk3.obj \
        $(SLO)$/undodat.obj \
        $(SLO)$/undodraw.obj \
        $(SLO)$/undotab.obj

EXCEPTIONSFILES= \
    $(SLO)$/undoblk3.obj \
    $(SLO)$/undocell.obj \
    $(SLO)$/undostyl.obj \
    $(SLO)$/undotab.obj

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

