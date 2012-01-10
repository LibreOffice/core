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

PRJNAME=sc
TARGET=tst
IDLMAP=map.idl

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

UNOUCRDEP=	$(BIN)$/applicat.rdb
UNOUCRRDB=	$(BIN)$/applicat.rdb

UNOTYPES=\
    com.sun.star.util.Date		\
    com.sun.star.sheet.XAddIn	\
    com.sun.star.sheet.XVolatileResult	\
    com.sun.star.lang.XServiceName	\
    com.sun.star.lang.XServiceInfo	\
    com.sun.star.lang.Locale	\
    com.sun.star.lang.XSingleServiceFactory	\
    com.sun.star.registry.XRegistryKey	\
    com.sun.star.uno.XWeak	\
    com.sun.star.uno.XAggregation	\
    com.sun.star.uno.TypeClass	\
    com.sun.star.lang.XTypeProvider	\
    com.sun.star.lang.XMultiServiceFactory	\
    stardiv.starcalc.test.XTestAddIn

PACKAGE=stardiv$/starcalc$/test
IDLFILES= testadd.idl

SLOFILES=	\
        $(SLO)$/addin.obj \
        $(SLO)$/result.obj

EXCEPTIONSFILES= \
        $(SLO)$/addin.obj \
        $(SLO)$/result.obj

SHL1TARGET= $(TARGET)$(DLLPOSTFIX)

SHL1STDLIBS=$(VCLLIB) $(TOOLSLIB) $(SVLLIB) $(TKLIB) $(SALLIB) $(VOSLIB) $(CPPULIB) $(CPPUHELPERLIB)

SHL1DEPN=   $(L)$/itools.lib $(SVLIBDEPEND)
SHL1DEF=    $(MISC)$/$(SHL1TARGET).def
SHL1IMPLIB=$(TARGET)
SHL1OBJS=	$(SLO)$/addin.obj $(SLO)$/result.obj $(SLO)$/workben$/testadd.obj

# --- Targets ------------------------------------------------------
.INCLUDE :  target.mk

.IF "$(GUI)" == "WNT"

$(MISC)$/$(SHL1TARGET).def:  makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo LIBRARY     $(SHL1TARGET)                                  >$@
.IF "$(COM)"!="GCC"
    @echo DESCRIPTION 'StarOne Test-DLL'                            >>$@
    @echo DATA        READ WRITE NONSHARED                          >>$@
.ENDIF
    @echo EXPORTS                                                   >>$@
    @echo   component_getImplementationEnvironment @24              >>$@
    @echo   component_writeInfo @25                                 >>$@
    @echo   component_getFactory @26                                >>$@
.ENDIF

