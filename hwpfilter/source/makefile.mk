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

PRJNAME=hwpfilter
TARGET=hwp

USE_DEFFILE=TRUE
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

.IF "$(SYSTEM_ZLIB)" == "YES"
CFLAGS+=-DSYSTEM_ZLIB
.ENDIF

SLOFILES =	\
        $(SLO)$/attributes.obj \
        $(SLO)$/cspline.obj \
        $(SLO)$/fontmap.obj \
        $(SLO)$/formula.obj \
        $(SLO)$/grammar.obj \
        $(SLO)$/hbox.obj \
        $(SLO)$/hcode.obj \
        $(SLO)$/hfont.obj \
        $(SLO)$/hgzip.obj \
        $(SLO)$/hinfo.obj \
        $(SLO)$/hiodev.obj \
        $(SLO)$/hpara.obj \
        $(SLO)$/hstream.obj \
        $(SLO)$/hstyle.obj \
        $(SLO)$/htags.obj \
        $(SLO)$/hutil.obj \
        $(SLO)$/hwpeq.obj \
        $(SLO)$/hwpfile.obj \
        $(SLO)$/hwpread.obj \
        $(SLO)$/hwpreader.obj \
        $(SLO)$/lexer.obj \
        $(SLO)$/mzstring.obj \
        $(SLO)$/solver.obj

# --- Shared-Library -----------------------------------------------

SHL1TARGET= $(TARGET)
#SHL1IMPLIB= ihwp

SHL1STDLIBS= \
        $(CPPULIB)		\
        $(CPPUHELPERLIB)\
        $(SALLIB)		\
        $(ZLIB3RDLIB)

.IF "$(GUI)"=="WNT"
SHL1STDLIBS+= \
        $(OLE32LIB) $(UUIDLIB)
.ENDIF			# "$(GUI)"=="WNT"		


SHL1DEF=    $(MISC)$/$(SHL1TARGET).def
SHL1LIBS=   $(SLB)$/$(TARGET).lib
SHL1VERSIONMAP = $(SOLARENV)/src/component.map

DEF1NAME=$(SHL1TARGET)

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

ALLTAR : $(MISC)/hwp.component

$(MISC)/hwp.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        hwp.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt hwp.component
