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
PRJNAME=filter
TARGET=svgfilter

ENABLE_EXCEPTIONS=TRUE
VISIBILITY_HIDDEN=TRUE

# --- Settings ----------------------------------

.INCLUDE :  	settings.mk

# --- Types -------------------------------------

SLOFILES=	$(SLO)$/svguno.obj			\
            $(SLO)$/svgdialog.obj		\
            $(SLO)$/impsvgdialog.obj	\
            $(SLO)$/svgfilter.obj		\
            $(SLO)$/svgexport.obj		\
            $(SLO)$/svgfontexport.obj	\
            $(SLO)$/svgwriter.obj	
.IF "$(SOLAR_JAVA)"!=""
SLOFILES+=		$(SLO)$/svgimport.obj
.ENDIF

# --- Library -----------------------------------

SHL1TARGET=$(TARGET)$(DLLPOSTFIX)

SHL1STDLIBS=\
    $(SVXCORELIB)			\
    $(EDITENGLIB)			\
    $(XMLOFFLIB)		\
    $(VCLLIB)			\
    $(UNOTOOLSLIB)		\
    $(TOOLSLIB)			\
    $(COMPHELPERLIB)	\
    $(CPPUHELPERLIB)	\
    $(CPPULIB)			\
    $(SALLIB) 			\
    $(BASEGFXLIB) 		\
    $(SVTOOLLIB)		

.IF "$(SOLAR_JAVA)"!=""
SHL1STDLIBS+=\
    $(JVMACCESSLIB)
.ENDIF


SHL1DEPN=
SHL1IMPLIB=	i$(SHL1TARGET)
SHL1LIBS=	$(SLB)$/$(TARGET).lib
SHL1VERSIONMAP=$(SOLARENV)/src/component.map
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def

DEF1NAME=$(SHL1TARGET)

# --- Targets ----------------------------------

.INCLUDE : target.mk

ALLTAR : $(MISC)/svgfilter.component

$(MISC)/svgfilter.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        svgfilter.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt svgfilter.component
