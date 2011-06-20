#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************
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

SHL1STDLIBS=$(VCLLIB) $(TOOLSLIB) $(SVLLIB) $(TKLIB) $(SALLIB)  $(CPPULIB) $(CPPUHELPERLIB)

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

