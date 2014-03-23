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
TARGET=docshell
LIBTARGET=no
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  scpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sc.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

CXXFILES = \
        docsh.cxx	\
        docsh2.cxx	\
        docsh3.cxx	\
        docsh4.cxx 	\
        docsh5.cxx	\
        docsh6.cxx 	\
        docsh7.cxx 	\
        docsh8.cxx 	\
        externalrefmgr.cxx \
        tablink.cxx 	\
        arealink.cxx 	\
        dbdocfun.cxx 	\
        dbdocimp.cxx 	\
        impex.cxx	\
        docfunc.cxx	\
        olinefun.cxx	\
        servobj.cxx	\
        tpstat.cxx	\
        autostyl.cxx	\
        pagedata.cxx \
        hiranges.cxx \
        pntlock.cxx \
        sizedev.cxx \
        editable.cxx


SLOFILES =  \
        $(SLO)$/docsh.obj	\
        $(SLO)$/docsh2.obj	\
        $(SLO)$/docsh3.obj	\
        $(SLO)$/docsh4.obj   	\
        $(SLO)$/docsh5.obj   	\
        $(SLO)$/docsh6.obj   	\
        $(SLO)$/docsh7.obj   	\
        $(SLO)$/docsh8.obj   	\
        $(SLO)$/externalrefmgr.obj \
        $(SLO)$/tablink.obj   	\
        $(SLO)$/arealink.obj   	\
        $(SLO)$/dbdocfun.obj 	\
        $(SLO)$/dbdocimp.obj 	\
        $(SLO)$/impex.obj	\
        $(SLO)$/docfunc.obj	\
        $(SLO)$/olinefun.obj	\
        $(SLO)$/servobj.obj	\
        $(SLO)$/tpstat.obj	\
        $(SLO)$/autostyl.obj	\
        $(SLO)$/pagedata.obj \
        $(SLO)$/hiranges.obj \
        $(SLO)$/pntlock.obj \
        $(SLO)$/sizedev.obj \
        $(SLO)$/editable.obj


EXCEPTIONSFILES= \
        $(SLO)$/docsh.obj \
        $(SLO)$/docsh3.obj	\
        $(SLO)$/docsh4.obj \
        $(SLO)$/docsh5.obj   	\
        $(SLO)$/docsh8.obj \
        $(SLO)$/externalrefmgr.obj \
        $(SLO)$/dbdocimp.obj \
        $(SLO)$/docfunc.obj

SRS1NAME=$(TARGET)
SRC1FILES =  tpstat.src

LIB1TARGET = $(SLB)$/$(TARGET).lib

LIB1OBJFILES =  \
        $(SLO)$/docsh.obj	\
        $(SLO)$/docsh2.obj	\
        $(SLO)$/docsh3.obj	\
        $(SLO)$/docsh4.obj   	\
        $(SLO)$/docsh5.obj   	\
        $(SLO)$/docsh6.obj   	\
        $(SLO)$/docsh7.obj   	\
        $(SLO)$/docsh8.obj   	\
        $(SLO)$/externalrefmgr.obj \
        $(SLO)$/tablink.obj   	\
        $(SLO)$/arealink.obj   	\
        $(SLO)$/dbdocfun.obj 	\
        $(SLO)$/dbdocimp.obj 	\
        $(SLO)$/impex.obj	\
        $(SLO)$/docfunc.obj	\
        $(SLO)$/olinefun.obj	\
        $(SLO)$/servobj.obj	\
        $(SLO)$/autostyl.obj	\
        $(SLO)$/pagedata.obj \
        $(SLO)$/hiranges.obj \
        $(SLO)$/pntlock.obj \
        $(SLO)$/sizedev.obj \
        $(SLO)$/editable.obj

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

