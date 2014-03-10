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
TARGET=drawfunc
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  scpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sc.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SRS1NAME=$(TARGET)
SRC1FILES =  \
        objdraw.src\
        drformsh.src

SLOFILES =  \
        $(SLO)$/fupoor.obj \
        $(SLO)$/fumark.obj \
        $(SLO)$/fudraw.obj \
        $(SLO)$/fusel.obj \
        $(SLO)$/fusel2.obj \
        $(SLO)$/fuconstr.obj \
        $(SLO)$/fuconrec.obj \
        $(SLO)$/fuconarc.obj \
        $(SLO)$/fuconuno.obj \
        $(SLO)$/fuconpol.obj \
        $(SLO)$/fuconcustomshape.obj \
        $(SLO)$/fuins1.obj \
        $(SLO)$/fuins2.obj \
        $(SLO)$/futext.obj \
        $(SLO)$/futext2.obj \
        $(SLO)$/futext3.obj \
        $(SLO)$/drawsh.obj \
        $(SLO)$/drawsh2.obj \
        $(SLO)$/drawsh4.obj \
        $(SLO)$/drawsh5.obj \
        $(SLO)$/drtxtob.obj \
        $(SLO)$/drtxtob1.obj \
        $(SLO)$/drtxtob2.obj \
        $(SLO)$/drformsh.obj \
        $(SLO)$/oleobjsh.obj \
        $(SLO)$/chartsh.obj  \
        $(SLO)$/graphsh.obj	\
        $(SLO)$/mediash.obj

EXCEPTIONSFILES= \
        $(SLO)$/fusel.obj \
        $(SLO)$/fuins2.obj \
        $(SLO)$/graphsh.obj	\
        $(SLO)$/mediash.obj

NOOPTFILES=\
    $(SLO)$/fusel.obj

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

