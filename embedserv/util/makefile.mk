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

PRJNAME=embedserv
TARGET=emser
LIBTARGET=NO
USE_DEFFILE=TRUE

# --- Settings -----------------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

.IF "$(GUI)" == "WNT" && "$(DISABLE_ATL)"==""

SHL1TARGET= emser$(DLLPOSTFIX)

SHL1IMPLIB= emserimp
SHL1OBJS= \
        $(SLO)$/register.obj \
        $(SLO)$/servprov.obj \
        $(SLO)$/docholder.obj \
        $(SLO)$/ed_ipersiststr.obj \
        $(SLO)$/ed_idataobj.obj \
        $(SLO)$/ed_ioleobject.obj \
        $(SLO)$/ed_iinplace.obj \
        $(SLO)$/iipaobj.obj \
        $(SLO)$/guid.obj \
        $(SLO)$/esdll.obj \
        $(SLO)$/intercept.obj \
        $(SLO)$/syswinwrapper.obj \
        $(SLO)$/tracker.obj

SHL1STDLIBS=\
        $(SALLIB)			\
        $(CPPULIB)			\
        $(CPPUHELPERLIB)	\
        $(OLE32LIB)			\
        $(GDI32LIB)			\
        $(UUIDLIB)				\
        $(OLEAUT32LIB)

.IF "$(COM)"=="MSC"
.IF "$(CCNUMVER)" >= "001300000000" && "$(CCNUMVER)" <= "001399999999"
SHL1STDLIBS+=\
        $(ADVAPI32LIB)	\
        $(ATL_LIB)$/atls.lib
.ENDIF

.IF "$(WINDOWS_VISTA_PSDK)"!=""
SHL1STDLIBS+=\
        $(ADVAPI32LIB)	\
        $(ATL_LIB)$/atls.lib
.ENDIF # "$(WINDOWS_VISTA_PSDK)"!=""
.ENDIF # "$(COM)"=="MSC"


SHL1DEF=		$(MISC)$/$(SHL1TARGET).def

DEF1NAME=		$(SHL1TARGET)
DEF1EXPORTFILE=	exports.dxp

.ENDIF

# --- Targets -------------------------------------------------------------

.INCLUDE :  target.mk



ALLTAR : $(MISC)/emser.component

$(MISC)/emser.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        emser.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt emser.component
