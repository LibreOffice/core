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



PRJ=				..$/..
PRJNAME=			chart2
TARGET=				chartview

USE_DEFFILE=		TRUE
ENABLE_EXCEPTIONS=	TRUE

# --- Settings -----------------------------------------------------

.INCLUDE: $(PRJ)$/util$/makefile.pmk
.INCLUDE: $(PRJ)$/chartview.pmk

# --- export library -------------------------------------------------

#You can use several library macros of this form to build libraries that
#do not consist of all object files in a directory or to merge different libraries.
LIB1TARGET=		$(SLB)$/$(TARGET).lib

LIB1FILES=		\
                $(SLB)$/chvaxes.lib \
                $(SLB)$/chvtypes.lib \
                $(SLB)$/chvdiagram.lib \
                $(SLB)$/chview.lib

#--------

#Indicates the filename of the shared library.
SHL1TARGET=		$(TARGET)$(DLLPOSTFIX)

#indicates dependencies:
.IF "$(COM)" == "MSC"
SHL1DEPN = \
        $(LB)$/icharttools.lib
.ELSE
SHL1DEPN =
.ENDIF

#Specifies an import library to create. For Win32 only.
SHL1IMPLIB=		i$(TARGET)

#Specifies libraries from the same module to put into the shared library.
#was created above
SHL1LIBS= 		$(LIB1TARGET)

#Links import libraries.

SHL1STDLIBS=	$(CHARTTOOLS)		\
                $(CPPULIB)			\
                $(EDITENGLIB)			\
                $(CPPUHELPERLIB)	\
                $(COMPHELPERLIB)	\
                $(SALLIB)			\
                $(SVLLIB)			\
                $(SVTOOLLIB)		\
                $(SVXCORELIB)			\
                $(TOOLSLIB) 		\
                $(UNOTOOLSLIB)      \
                $(BASEGFXLIB) 		\
                $(VCLLIB)           \
                $(SFXLIB)			\
                $(BASEGFXLIB)

#--------exports

#specifies the exported symbols for Windows only:
SHL1DEF=		$(MISC)$/$(SHL1TARGET).def

#Specifies the library name to parse for symbols. For Win32 only.
DEFLIB1NAME=	$(TARGET)

#A file of symbols to export.
#DEF1EXPORTFILE=	$(PRJ)$/source$/inc$/exports.dxp

#--------definition file

#name of the definition file:
DEF1NAME=		$(SHL1TARGET)

# indicates definition file dependencies
DEF1DEPN=		$(MISC)$/$(SHL1TARGET).flt

#A comment on the definition file.
DEF1DES=		Viewable Component Chart View

# --- Targets -----------------------------------------------------------------

.INCLUDE: target.mk

# --- Filter -----------------------------------------------------------

$(MISC)$/$(SHL1TARGET).flt: makefile.mk \
                            exports.flt
    $(TYPE) exports.flt > $@

ALLTAR : $(MISC)/chartview.component

$(MISC)/chartview.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        chartview.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt chartview.component
