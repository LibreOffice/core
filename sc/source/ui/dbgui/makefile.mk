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
TARGET=dbgui
LIBTARGET=no
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  scpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sc.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SLOFILES =	\
    $(SLO)$/sortdlg.obj		\
    $(SLO)$/tpsort.obj		\
    $(SLO)$/filtdlg.obj		\
    $(SLO)$/sfiltdlg.obj	\
    $(SLO)$/foptmgr.obj		\
    $(SLO)$/pfiltdlg.obj	\
    $(SLO)$/dbnamdlg.obj	\
    $(SLO)$/expftext.obj	\
    $(SLO)$/textimportoptions.obj \
    $(SLO)$/subtdlg.obj		\
    $(SLO)$/tpsubt.obj		\
    $(SLO)$/fieldwnd.obj	\
    $(SLO)$/pvlaydlg.obj	\
    $(SLO)$/pvfundlg.obj	\
    $(SLO)$/dpgroupdlg.obj	\
    $(SLO)$/dapitype.obj	\
    $(SLO)$/dapidata.obj	\
    $(SLO)$/consdlg.obj		\
    $(SLO)$/scendlg.obj		\
    $(SLO)$/imoptdlg.obj	\
    $(SLO)$/validate.obj	\
    $(SLO)$/csvsplits.obj	\
    $(SLO)$/csvcontrol.obj	\
    $(SLO)$/csvruler.obj	\
    $(SLO)$/csvgrid.obj		\
    $(SLO)$/csvtablebox.obj	\
    $(SLO)$/asciiopt.obj	\
    $(SLO)$/scuiasciiopt.obj	\
     $(SLO)$/scuiimoptdlg.obj

EXCEPTIONSFILES= \
    $(SLO)$/csvgrid.obj \
    $(SLO)$/csvruler.obj \
    $(SLO)$/csvsplits.obj \
    $(SLO)$/csvtablebox.obj \
    $(SLO)$/fieldwnd.obj \
    $(SLO)$/pvfundlg.obj	\
    $(SLO)$/pvlaydlg.obj	\
    $(SLO)$/dapidata.obj	\
    $(SLO)$/validate.obj

SRS1NAME=$(TARGET)
SRC1FILES =  \
    textimportoptions.src		\
        pivot.src		\
        pvfundlg.src	\
        dpgroupdlg.src	\
        dapitype.src	\
        consdlg.src		\
        scendlg.src     \
        imoptdlg.src    \
        validate.src    \
        asciiopt.src    \
        outline.src

LIB1TARGET = $(SLB)$/$(TARGET).lib

LIB1OBJFILES =  \
    $(SLO)$/filtdlg.obj		\
    $(SLO)$/sfiltdlg.obj	\
    $(SLO)$/foptmgr.obj		\
    $(SLO)$/dbnamdlg.obj	\
    $(SLO)$/expftext.obj	\
    $(SLO)$/fieldwnd.obj	\
    $(SLO)$/pvlaydlg.obj	\
    $(SLO)$/consdlg.obj		\
    $(SLO)$/imoptdlg.obj	\
    $(SLO)$/csvsplits.obj	\
    $(SLO)$/csvcontrol.obj	\
    $(SLO)$/csvruler.obj	\
    $(SLO)$/csvgrid.obj		\
    $(SLO)$/csvtablebox.obj	\
    $(SLO)$/asciiopt.obj

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk


