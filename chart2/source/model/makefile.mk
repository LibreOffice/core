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
TARGET=				chartmodel

USE_DEFFILE=		TRUE
ENABLE_EXCEPTIONS=	TRUE
VISIBILITY_HIDDEN=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE: $(PRJ)$/util$/makefile.pmk

# --- export library -------------------------------------------------

#You can use several library macros of this form to build libraries that
#do not consist of all object files in a directory or to merge different libraries.
LIB1TARGET=		$(SLB)$/_$(TARGET).lib

LIB1FILES=		\
                $(SLB)$/chmodelmain.lib \
                $(SLB)$/chmodeltemplate.lib \
                $(SLB)$/chmodelfilter.lib

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

SHL1STDLIBS=	$(CHARTTOOLS) 		\
                $(CPPULIB)			\
                $(CPPUHELPERLIB)	\
                $(COMPHELPERLIB)	\
                $(UNOTOOLSLIB)		\
                $(VCLLIB)			\
                $(SVLLIB)			\
                $(SVTOOLLIB)		\
                $(SALLIB)			\
                $(UCBHELPERLIB)     \
                $(FWELIB)           \

#--------exports

#specifies the exported symbols for Windows only:
SHL1DEF=		$(MISC)$/$(SHL1TARGET).def

SHL1VERSIONMAP = $(SOLARENV)/src/component.map

#--------definition file

#name of the definition file:
DEF1NAME=		$(SHL1TARGET)

# --- Resources ---------------------------------------------------------------

# --- Targets -----------------------------------------------------------------

.INCLUDE: target.mk

ALLTAR : $(MISC)/chartmodel.component

$(MISC)/chartmodel.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        chartmodel.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt chartmodel.component
