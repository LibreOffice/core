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
TARGET=unoobj

LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  scpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sc.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------
SLO1FILES =  \
        $(SLO)$/docuno.obj \
        $(SLO)$/servuno.obj \
        $(SLO)$/defltuno.obj \
        $(SLO)$/drdefuno.obj \
        $(SLO)$/cellsuno.obj \
        $(SLO)$/tokenuno.obj \
        $(SLO)$/textuno.obj \
        $(SLO)$/notesuno.obj \
        $(SLO)$/cursuno.obj \
        $(SLO)$/srchuno.obj \
        $(SLO)$/fielduno.obj \
        $(SLO)$/miscuno.obj \
        $(SLO)$/optuno.obj \
        $(SLO)$/appluno.obj \
        $(SLO)$/funcuno.obj \
        $(SLO)$/nameuno.obj \
        $(SLO)$/viewuno.obj \
        $(SLO)$/dispuno.obj \
        $(SLO)$/datauno.obj \
        $(SLO)$/dapiuno.obj \
        $(SLO)$/chartuno.obj \
        $(SLO)$/chart2uno.obj \
        $(SLO)$/shapeuno.obj \
        $(SLO)$/pageuno.obj \
        $(SLO)$/forbiuno.obj \
        $(SLO)$/styleuno.obj \
        $(SLO)$/afmtuno.obj \
        $(SLO)$/fmtuno.obj \
        $(SLO)$/linkuno.obj \
        $(SLO)$/targuno.obj \
        $(SLO)$/convuno.obj \
        $(SLO)$/editsrc.obj \
        $(SLO)$/unoguard.obj \
        $(SLO)$/confuno.obj \
        $(SLO)$/filtuno.obj \
        $(SLO)$/unodoc.obj \
        $(SLO)$/addruno.obj \
        $(SLO)$/eventuno.obj \
        $(SLO)$/listenercalls.obj \
        $(SLO)$/cellvaluebinding.obj \
        $(SLO)$/celllistsource.obj \
        $(SLO)$/warnpassword.obj \
        $(SLO)$/unoreflist.obj \
        $(SLO)$/ChartRangeSelectionListener.obj

SLO2FILES = \
    $(SLO)$/scdetect.obj								\
    $(SLO)$/detreg.obj

SLOFILES = \
    $(SLO1FILES) \
    $(SLO2FILES)

LIB1TARGET = \
    $(SLB)$/$(TARGET).lib

LIB1OBJFILES = \
    $(SLO1FILES)

.IF "$(OS)$(COM)$(CPUNAME)"=="LINUXGCCSPARC"
NOOPTFILES= \
             $(SLO)$/cellsuno.obj
.ENDIF

# Work around bug in gcc 4.2 / 4.3, see
# http://gcc.gnu.org/bugzilla/show_bug.cgi?id=35182
.IF "$(COM)"=="GCC"
NOOPTFILES+= \
    $(SLO)$/chart2uno.obj
.ENDIF


# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

