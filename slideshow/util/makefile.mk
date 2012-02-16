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

PRJNAME=slideshow
TARGET=slideshow
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------------

.INCLUDE :	settings.mk

# --- Common ----------------------------------------------------------

.IF "$(VERBOSE)"=="TRUE"
CDEFS+=-DVERBOSE
.ENDIF

SHL1DLLPRE=
LIB1TARGET=$(SLB)$/$(TARGET).lib
LIB1FILES=\
    $(SLB)$/transitions.lib	\
    $(SLB)$/activities.lib	\
    $(SLB)$/animationnodes.lib	\
    $(SLB)$/shapes.lib \
    $(SLB)$/slide.lib \
    $(SLB)$/engine.lib

.IF "$(GUI)" == "OS2"
SHL1TARGET=slidesho
.ELSE
SHL1TARGET=$(TARGET).uno
.ENDIF

SHL1STDLIBS= $(TOOLSLIB) \
             $(CPPULIB) \
             $(SALLIB) \
             $(VCLLIB) \
             $(COMPHELPERLIB) \
             $(CPPUHELPERLIB) \
             $(BASEGFXLIB) \
             $(CANVASTOOLSLIB) \
             $(CPPCANVASLIB) \
             $(UNOTOOLSLIB) \
             $(SVTOOLLIB) \
             $(AVMEDIALIB)

SHL1IMPLIB=i$(TARGET)
SHL1LIBS=$(SLB)$/$(TARGET).lib
SHL1DEF=$(MISC)$/$(SHL1TARGET).def

SHL1VERSIONMAP=$(SOLARENV)/src/component.map

DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=exports.dxp

# ==========================================================================

.IF "$(GUI)" == "OS2"
TARGET2=slshtest
.ELSE
TARGET2=slideshowtest
.ENDIF
LIB2TARGET=$(SLB)$/$(TARGET2).lib
LIB2FILES=$(LIB1FILES)

SHL2TARGET=$(TARGET2)$(DLLPOSTFIX)
SHL2IMPLIB=i$(TARGET2)
SHL2STDLIBS= $(SHL1STDLIBS)

SHL2DEF=$(MISC)$/$(SHL2TARGET).def
SHL2LIBS=$(SLB)$/$(TARGET2).lib

DEF2NAME	=$(SHL2TARGET)
DEF2LIBNAME = $(TARGET2)
.IF "$(GUI)" == "OS2"
DEF2DEPN	=$(MISC)$/slideshowtest.flt
.ELSE
DEF2DEPN	=$(MISC)$/$(SHL2TARGET).flt
.ENDIF

DEF2DES		=SlideShowTest

# ==========================================================================

.INCLUDE :	target.mk

.IF "$(GUI)" == "OS2"
$(MISC)$/slideshowtest.flt: makefile.mk \
                            slideshowtest.flt
    +$(TYPE) slideshowtest.flt > $@
.ELSE
$(MISC)$/$(SHL2TARGET).flt: makefile.mk \
                            $(TARGET2).flt
    +$(TYPE) $(TARGET2).flt > $@

.ENDIF

ALLTAR : $(MISC)/slideshow.component

$(MISC)/slideshow.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        slideshow.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt slideshow.component
