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

PRJ=..

PRJNAME=starmath
TARGET=starmath

LIBTARGET=NO

# --- Settings -----------------------------------------------------

ENABLE_EXCEPTIONS=TRUE

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

SMDLL=TRUE

SRS2NAME =smres
SRC2FILES = smres.src   \
            symbol.src	\
            toolbox.src \
            commands.src

SLO1FILES =  \
        $(SLO)$/accessibility.obj \
        $(SLO)$/action.obj \
        $(SLO)$/cfgitem.obj \
        $(SLO)$/config.obj \
        $(SLO)$/dialog.obj \
        $(SLO)$/document.obj \
        $(SLO)$/edit.obj \
        $(SLO)$/eqnolefilehdr.obj \
        $(SLO)$/format.obj \
        $(SLO)$/mathmlexport.obj \
        $(SLO)$/mathmlimport.obj \
        $(SLO)$/mathtype.obj \
        $(SLO)$/node.obj \
        $(SLO)$/parse.obj \
        $(SLO)$/rect.obj \
        $(SLO)$/register.obj \
        $(SLO)$/smdll.obj \
        $(SLO)$/smmod.obj \
        $(SLO)$/symbol.obj \
        $(SLO)$/toolbox.obj \
        $(SLO)$/typemap.obj \
        $(SLO)$/types.obj \
        $(SLO)$/unodoc.obj \
        $(SLO)$/utility.obj \
        $(SLO)$/view.obj \
        $(SLO)$/unomodel.obj

SLO2FILES =  \
        $(SLO)$/register.obj  \
        $(SLO)$/detreg.obj

SLOFILES = \
    $(SLO)$/smdetect.obj \
    $(SLO1FILES) \
    $(SLO2FILES)

LIB1TARGET = \
    $(SLB)$/$(TARGET).lib

LIB1OBJFILES = \
    $(SLO1FILES)

# --- Targets -------------------------------------------------------

.INCLUDE :  target.mk

$(INCCOM)$/dllname.hxx: makefile.mk
.IF "$(GUI)"=="UNX"
    $(RM) $@
    echo \#define DLL_NAME \"$(DLLPRE)sm$(DLLPOSTFIX)$(DLLPOST)\" >$@
.ELSE
    echo $(EMQ)#define DLL_NAME $(EMQ)"$(DLLPRE)sm$(DLLPOSTFIX)$(DLLPOST)$(EMQ)" >$@
.ENDIF

$(SRS)$/smres.srs: $(SOLARINCDIR)$/svx$/globlmn.hrc


$(SRS)$/$(SRS2NAME).srs  : $(LOCALIZE_ME_DEST)

