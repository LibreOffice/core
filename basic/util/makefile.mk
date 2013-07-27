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

PRJNAME=basic
TARGET=sb
#basic.hid generieren
GEN_HID=TRUE

# --- Settings ---------------------------------------------------

.INCLUDE :  settings.mk

# --- Allgemein ---------------------------------------------------

LIB1TARGET=$(SLB)$/sb.lib
LIB1FILES=		\
    $(SLB)$/basicmgr.lib \
    $(SLB)$/classes.lib \
    $(SLB)$/comp.lib \
    $(SLB)$/uno.lib \
    $(SLB)$/runtime.lib \
    $(SLB)$/sbx.lib

SHL1TARGET= sb$(DLLPOSTFIX)
SHL1IMPLIB= basic

SHL1STDLIBS= \
            $(CPPULIB) \
            $(CPPUHELPERLIB) \
            $(TOOLSLIB) \
            $(SVTOOLLIB) \
            $(SVLLIB)	\
            $(VCLLIB) \
            $(VOSLIB) \
            $(SALLIB) \
            $(SALHELPERLIB) \
            $(COMPHELPERLIB) \
            $(UNOTOOLSLIB) \
            $(SOTLIB) \
            $(VOSLIB) \
            $(XMLSCRIPTLIB)

# Uncomment the following line if DBG_TRACE_PROFILING is active in source/inc/sbtrace.hxx
# SHL1STDLIBS+=$(CANVASTOOLSLIB)

.IF "$(SOLAR_JAVA)" != "TRUE"
SHL1STDLIBS+=$(SJLIB)
.ENDIF

.IF "$(GUI)"=="WNT"
SHL1STDLIBS+=	\
    $(UWINAPILIB)	\
    $(OLEAUT32LIB)
.ENDIF # WNT

.IF "$(GUI)" != "UNX"
.IF "$(COM)" != "GCC"
SHL1OBJS=	\
    $(SLO)$/sb.obj
.ENDIF
.ENDIF

SHL1DEF=	$(MISC)$/$(SHL1TARGET).def
SHL1LIBS=	$(SLB)$/sb.lib

DEF1NAME	=$(SHL1TARGET)
DEF1DEPN	=	\
    $(MISC)$/$(SHL1TARGET).flt

DEFLIB1NAME	=sb
DEF1DES		=StarBasic


RES1TARGET=$(PRJNAME)
SRS1FILES= \
        $(SRS)$/classes.srs \
        $(SRS)$/sbx.srs

RESLIB2NAME=sb
RESLIB2IMAGES=$(PRJ)$/res
RESLIB2SRSFILES= \
        $(SRS)$/classes.srs \
        $(SRS)$/sbx.srs
########## remove sbx.srx (and this line) when issue i53795 is fixed

# --- Targets -----------------------------------------------------------

.INCLUDE :  target.mk
 
#-------------------------------------------------------------------------
#								Windows NT
#-------------------------------------------------------------------------
#
#					default targets aus target.mk
#

# --- Basic-Filter-Datei ---

$(MISC)$/$(SHL1TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo WEP > $@
    @echo LIBMAIN >> $@
    @echo LibMain >> $@
    @echo Sbi >> $@
    @echo SvRTL >> $@
    @echo SbRtl_ >> $@
    @echo exception >> $@
    @echo bad_alloc >> $@
    @echo __CT >> $@

$(SRS)$/basic.srs:
    $(TYPE) $(SRS)$/classes.srs + $(SRS)$/runtime.srs + $(SRS)$/sbx.srs > $@

ALLTAR : $(MISC)/sb.component

$(MISC)/sb.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        sb.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt sb.component
