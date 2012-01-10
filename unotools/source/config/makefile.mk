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
PRJINC=..$/..$/inc
PRJNAME=unotools
TARGET=config

ENABLE_EXCEPTIONS=TRUE

# --- Settings ----------------------------------

.INCLUDE : settings.mk
.INCLUDE :      $(PRJ)$/util$/makefile.pmk


# --- Files -------------------------------------

SLOFILES=\
        $(SLO)$/syslocaleoptions.obj \
        $(SLO)$/configvaluecontainer.obj \
        $(SLO)$/confignode.obj \
        $(SLO)$/configitem.obj \
        $(SLO)$/configmgr.obj  \
        $(SLO)$/configpathes.obj  \
        $(SLO)$/docinfohelper.obj  \
                $(SLO)$/bootstrap.obj \
        $(SLO)$/accelcfg.obj                                \
        $(SLO)$/cacheoptions.obj            \
    $(SLO)$/cmdoptions.obj				\
    $(SLO)$/compatibility.obj           \
    $(SLO)$/defaultoptions.obj			\
    $(SLO)$/dynamicmenuoptions.obj		\
    $(SLO)$/eventcfg.obj                \
    $(SLO)$/extendedsecurityoptions.obj \
        $(SLO)$/fltrcfg.obj                             \
    $(SLO)$/fontcfg.obj \
    $(SLO)$/fontoptions.obj				\
        $(SLO)$/historyoptions.obj                         \
    $(SLO)$/inetoptions.obj				\
    $(SLO)$/internaloptions.obj			\
        $(SLO)$/itemholder1.obj \
        $(SLO)$/javaoptions.obj             \
        $(SLO)$/lingucfg.obj                \
    $(SLO)$/localisationoptions.obj		\
        $(SLO)$/misccfg.obj                             \
        $(SLO)$/moduleoptions.obj           \
        $(SLO)$/options.obj                     \
        $(SLO)$/optionsdlg.obj              \
        $(SLO)$/pathoptions.obj             \
    $(SLO)$/printwarningoptions.obj		\
    $(SLO)$/regoptions.obj				\
    $(SLO)$/saveopt.obj					\
    $(SLO)$/searchopt.obj				\
    $(SLO)$/securityoptions.obj			\
    $(SLO)$/sourceviewconfig.obj		\
    $(SLO)$/startoptions.obj			\
        $(SLO)$/undoopt.obj                 \
        $(SLO)$/useroptions.obj      \
    $(SLO)$/viewoptions.obj				\
    $(SLO)$/workingsetoptions.obj		\
        $(SLO)$/xmlaccelcfg.obj

# --- Targets ----------------------------------

.INCLUDE : target.mk

