#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.7 $
#
#   last change: $Author: obo $ $Date: 2008-02-25 17:13:12 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
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

