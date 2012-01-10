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

PRJNAME=linguistic
TARGET=lng
ENABLE_EXCEPTIONS=sal_True

#----- Settings ---------------------------------------------------------

.INCLUDE : settings.mk

# --- Files --------------------------------------------------------

SLOFILES = \
        $(SLO)$/convdiclist.obj\
        $(SLO)$/convdic.obj\
        $(SLO)$/convdicxml.obj\
        $(SLO)$/dicimp.obj\
        $(SLO)$/dlistimp.obj\
        $(SLO)$/hhconvdic.obj\
        $(SLO)$/hyphdsp.obj\
        $(SLO)$/hyphdta.obj\
        $(SLO)$/iprcache.obj\
        $(SLO)$/lngopt.obj\
        $(SLO)$/lngprophelp.obj\
        $(SLO)$/lngreg.obj\
        $(SLO)$/lngsvcmgr.obj\
        $(SLO)$/misc.obj\
        $(SLO)$/misc2.obj\
        $(SLO)$/spelldsp.obj\
        $(SLO)$/spelldta.obj\
        $(SLO)$/thesdsp.obj\
        $(SLO)$/thesdta.obj\
        $(SLO)$/gciterator.obj

SHL1TARGET= $(TARGET)$(DLLPOSTFIX)

SHL1STDLIBS= \
        $(CPPULIB) 	 \
        $(CPPUHELPERLIB) 	 \
        $(COMPHELPERLIB)   \
        $(VOSLIB)		\
        $(TOOLSLIB)		\
        $(I18NISOLANGLIB) \
        $(SVLLIB)	\
        $(SALLIB)		\
        $(XMLOFFLIB)    \
        $(UCBHELPERLIB)	\
        $(UNOTOOLSLIB)  \
        $(ICUUCLIB)
        
# build DLL
SHL1LIBS=		$(SLB)$/$(TARGET).lib
SHL1DEPN=		$(SLB)$/$(TARGET).lib
SHL1DEF=		$(MISC)$/$(SHL1TARGET).def
#SHL1VERSIONMAP= $(TARGET).map
SHL1IMPLIB= ilng

# build DEF file
DEF1NAME	=$(SHL1TARGET)
DEF1DEPN    =$(MISC)$/$(SHL1TARGET).flt
DEFLIB1NAME =$(TARGET)
DEF1DES     =Linguistic main DLL
#DEF1EXPORTFILE=	exports.dxp


.IF "$(GUI)"=="WNT"
DEF1EXPORT1 = component_getFactory
DEF1EXPORT2 = component_getImplementationEnvironment
.ENDIF

# --- Targets ------------------------------------------------------


.INCLUDE : target.mk


$(MISC)$/$(SHL1TARGET).flt: makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo lcl > $@
    @echo component >> $@
    @echo __CT >> $@


ALLTAR : $(MISC)/lng.component

$(MISC)/lng.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        lng.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt lng.component
