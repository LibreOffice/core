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



#MKDEPENDSOLVER=YES

PRJ=..$/..

PRJNAME=embedserv
TARGET=emser


# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
#.INCLUDE :  $(PRJ)$/util$/makefile.pmk

.IF "$(DISABLE_ATL)"==""

LIBTARGET=NO
USE_DEFFILE=NO

INCPRE+= $(ATL_INCLUDE) 

# --- Files --------------------------------------------------------

SLOFILES =  \
        $(SLO)$/register.obj       \
        $(SLO)$/servprov.obj       \
        $(SLO)$/docholder.obj      \
        $(SLO)$/ed_ipersiststr.obj \
        $(SLO)$/ed_idataobj.obj    \
        $(SLO)$/ed_ioleobject.obj  \
        $(SLO)$/ed_iinplace.obj    \
        $(SLO)$/iipaobj.obj        \
        $(SLO)$/guid.obj           \
        $(SLO)$/esdll.obj          \
        $(SLO)$/intercept.obj      \
        $(SLO)$/syswinwrapper.obj  \
        $(SLO)$/tracker.obj

EXCEPTIONSFILES= \
        $(SLO)$/register.obj       \
        $(SLO)$/docholder.obj      \
        $(SLO)$/ed_ipersiststr.obj \
        $(SLO)$/ed_idataobj.obj    \
        $(SLO)$/ed_iinplace.obj    \
        $(SLO)$/ed_ioleobject.obj  \
        $(SLO)$/iipaobj.obj        \
        $(SLO)$/intercept.obj      \
        $(SLO)$/syswinwrapper.obj  \
        $(SLO)$/tracker.obj

.ENDIF
# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

