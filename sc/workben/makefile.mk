#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: ihi $ $Date: 2006-12-19 13:28:19 $
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

.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk

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

CXXFILES=	\
        addin.cxx \
        result.cxx

SLOFILES=	\
        $(SLO)$/addin.obj \
        $(SLO)$/result.obj

EXCEPTIONSFILES= \
        $(SLO)$/addin.obj \
        $(SLO)$/result.obj


#SRC1FILES=
#SRS1NAME=
#SRSFILES=      $(SRS)$/
#SRC2FILES=
#SRS2NAME=

#RESLIB1NAME=tst
#RESLIB1SRSFILES=\
#                $(SRS)$/
#                $(SOLARRESDIR)$/

SHL1TARGET= $(TARGET)$(UPD)$(DLLPOSTFIX)
#SHL1LIBS=$(SLB)$/$(TARGET).lib

SHL1STDLIBS=$(USRLIB) $(SVLIB) $(TOOLSLIB) $(ONELIB) $(SVLLIB) $(TKLIB) $(SALLIB) $(VOSLIB) $(CPPULIB) $(CPPUHELPERLIB)

SHL1DEPN=   $(L)$/itools.lib $(SVLIBDEPEND)
SHL1DEF=    $(MISC)$/$(SHL1TARGET).def
SHL1IMPLIB=$(TARGET)
#SHL1RES=    $(RES)$/$(TARGET).res
SHL1OBJS=	$(SLO)$/addin.obj $(SLO)$/result.obj $(SLO)$/workben$/testadd.obj

# --- Targets ------------------------------------------------------
.INCLUDE :  target.mk

.IF "$(GUI)" == "WNT"

$(MISC)$/$(SHL1TARGET).def:  makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo LIBRARY     $(SHL1TARGET)                                  >$@
    @echo DESCRIPTION 'StarOne Test-DLL'                            >>$@
    @echo DATA        READ WRITE NONSHARED                          >>$@
    @echo EXPORTS                                                   >>$@
    @echo   component_getImplementationEnvironment @24              >>$@
    @echo   component_writeInfo @25                                 >>$@
    @echo   component_getFactory @26                                >>$@
.ENDIF

.IF "$(GUI)" == "WIN"

$(MISC)$/$(SHL1TARGET).def:  makefile
    @echo ------------------------------
    @echo Making: $@
    @echo LIBRARY     $(SHL1TARGET)                                  >$@
    @echo DESCRIPTION 'StarOne Test-DLL'                                 >>$@
    @echo EXETYPE     WINDOWS                                       >>$@
    @echo PROTMODE                                                  >>$@
    @echo CODE        LOADONCALL MOVEABLE DISCARDABLE               >>$@
    @echo DATA        PRELOAD MOVEABLE SINGLE                       >>$@
    @echo HEAPSIZE    0                                             >>$@
    @echo EXPORTS                                                   >>$@
    @echo _CreateWindow @2 								>>$@
.ENDIF

.IF "$(GUI)" == "OS2"

$(MISC)$/$(SHL1TARGET).def:  makefile
    @echo ================================================================
    @echo building $@
    @echo ----------------------------------------------------------------
.IF "$(COM)"!="WTC"
    echo  LIBRARY		INITINSTANCE TERMINSTANCE			>$@
    echo  DESCRIPTION	'StarOne Test-DLL'           					>>$@
    echo  PROTMODE										   >>$@
        @echo CODE        LOADONCALL 			              >>$@
    @echo DATA		  PRELOAD MULTIPLE NONSHARED					  >>$@
        @echo EXPORTS                                                   >>$@
.IF "$(COM)"!="ICC"
    @echo _CreateWindow @2 								>>$@
.ELSE
    @echo CreateWindow @2 								>>$@
.ENDIF

.ELSE
        @echo option DESCRIPTION 'StarOne Test-DLL'		                    >$@
        @echo name $(BIN)$/$(SHL1TARGET).dll                         >>$@
    @echo CreateWindow_ @2 								>>$@
    @gawk -f s:\util\exp.awk temp.def				>>$@
    del temp.def
.ENDIF
.ENDIF
