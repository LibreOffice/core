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
TARGET=msfilter

ENABLE_EXCEPTIONS=TRUE
LIBTARGET=NO

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  makefile.pmk

# --- Files --------------------------------------------------------

SLOFILES= \
    $(SLO)$/countryid.obj		\
    $(SLO)$/escherex.obj		\
    $(SLO)$/eschesdo.obj		\
    $(SLO)$/dffrecordheader.obj \
    $(SLO)$/dffpropset.obj		\
    $(SLO)$/mscodec.obj			\
    $(SLO)$/msdffimp.obj		\
    $(SLO)$/msfiltertracer.obj	\
    $(SLO)$/msocximex.obj		\
    $(SLO)$/msoleexp.obj		\
    $(SLO)$/msvbahelper.obj		\
    $(SLO)$/msvbasic.obj		\
    $(SLO)$/services.obj		\
    $(SLO)$/svdfppt.obj			\
    $(SLO)$/svxmsbas.obj		\
    $(SLO)$/svxmsbas2.obj

SHL1TARGET= msfilter$(DLLPOSTFIX)
SHL1IMPLIB=	i$(TARGET)
SHL1OBJS=	$(SLOFILES)
SHL1USE_EXPORTS=name
SHL1STDLIBS= \
             $(EDITENGLIB) \
             $(SVXCORELIB) \
             $(SFX2LIB) \
             $(XMLOFFLIB) \
             $(BASEGFXLIB) \
             $(BASICLIB) \
             $(SVTOOLLIB) \
             $(TKLIB) \
             $(VCLLIB) \
             $(SVLLIB) \
             $(SOTLIB) \
             $(UNOTOOLSLIB) \
             $(TOOLSLIB) \
             $(XMLSCRIPTLIB) \
             $(COMPHELPERLIB) \
             $(CPPUHELPERLIB) \
             $(CPPULIB) \
             $(SALLIB)

SHL1DEF=	$(MISC)$/$(SHL1TARGET).def
DEF1NAME=	$(SHL1TARGET)

.INCLUDE :  target.mk

ALLTAR : $(MISC)/msfilter.component

$(MISC)/msfilter.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        msfilter.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt msfilter.component
