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

EXTERNAL_WARNINGS_NOT_ERRORS := TRUE
PRJ=..$/..
BFPRJ=..

PRJNAME=binfilter
TARGET=scalc3

NO_HIDS=TRUE

# --- Settings -----------------------------------------------------------

.INCLUDE :  settings.mk
INC+= -I$(PRJ)$/inc$/bf_sc
IENV!:=$(IENV);..$/res

# --- Resourcen ----------------------------------------------------

RESLIB1LIST=\
    $(SRS)$/sc_ui.srs		\
    $(SRS)$/sc_dbgui.srs	\
    $(SRS)$/sc_core.srs 	\

RESLIB1NAME=bf_sc
RESLIB1SRSFILES=\
    $(RESLIB1LIST)

# --- StarClac DLL

SHL1TARGET= bf_sc$(DLLPOSTFIX)
SHL1VERSIONMAP= bf_sc.map
SHL1IMPLIB= bf_sci

.IF "$(OS)" != "MACOSX"
# static libraries
SHL1STDLIBS+= $(BFSCHLIB)
.ENDIF

# dynamic libraries
SHL1STDLIBS+=       \
    $(BFBASICLIB)		\
       $(LEGACYSMGRLIB)	\
    $(BFSO3LIB)		\
    $(BFSVTOOLLIB)	\
    $(BFSVXLIB)		\
    $(BFOFALIB)		\
    $(VCLLIB)		\
    $(CPPULIB)		\
    $(CPPUHELPERLIB)	\
    $(COMPHELPERLIB)	\
    $(UCBHELPERLIB)	\
    $(SALLIB)		\
    $(SALHELPERLIB)		\
    $(TOOLSLIB)		\
    $(I18NISOLANGLIB)   \
    $(UNOTOOLSLIB)  \
    $(SOTLIB)		\
    $(BFXMLOFFLIB)

.IF "$(OS)" == "MACOSX"
# static libraries must come at the end of list on MacOSX
SHL1STDLIBS+= $(BFSCHLIB)
.ENDIF


SHL1LIBS=   $(LIB3TARGET) $(LIB4TARGET)

.IF "$(GUI)"!="UNX"
.IF "$(GUI)$(COM)" != "WNTGCC"
SHL1OBJS=   $(SLO)$/sc_scdll.obj
.ENDIF
.ENDIF


SHL1DEF=    $(MISC)$/$(SHL1TARGET).def

.IF "$(GUI)"=="WNT"
SHL1RES=    $(RCTARGET)
.ENDIF

# --- Linken der Applikation ---------------------------------------

LIB2TARGET=$(SLB)$/scmod.lib
LIB2OBJFILES=	\
            $(SLO)$/sc_scmod.obj

LIB3TARGET=$(SLB)$/scalc3.lib

LIB3FILES=	\
    $(SLB)$/sc_app.lib \
    $(SLB)$/sc_docshell.lib \
    $(SLB)$/sc_view.lib \
    $(SLB)$/sc_dbgui.lib

LIB3FILES+= \
            $(SLB)$/sc_unoobj.lib

LIB4TARGET=$(SLB)$/scalc3c.lib

LIB4FILES=	\
    $(SLB)$/sc_data.lib \
    $(SLB)$/sc_tool.lib \
    $(SLB)$/sc_xml.lib

LIB5TARGET=$(LB)$/bf_sclib.lib
LIB5ARCHIV=$(LB)$/libbf_sclib.a

LIB5OBJFILES=$(SLO)$/sc_sclib.obj

# --- Targets -------------------------------------------------------------

.INCLUDE :  target.mk

ALLTAR:	\
    $(MISC)$/linkinc.ls

.IF "$(GUI)" == "WNT"

$(MISC)$/$(SHL1TARGET).def:  makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo LIBRARY     $(SHL1TARGET)                                  >$@
.IF "$(COM)"!="GCC"
    @echo DESCRIPTION 'SCALC3 DLL'                                 >>$@
    @echo DATA        READ WRITE NONSHARED                          >>$@
.ENDIF
    @echo EXPORTS                                                   >>$@
    @echo   CreateScDocShellDll @20                            >>$@
    @echo   CreateObjScDocShellDll @21                         >>$@
    @echo   InitScDll @22                                          >>$@
    @echo   DeInitScDll @23                                        >>$@
    @echo   component_getImplementationEnvironment @24             >>$@
    @echo   component_writeInfo @25                                >>$@
    @echo   component_getFactory @26                               >>$@
.ENDIF
.IF "$(OPTLINKS)" == "YES"
    echo  RC $(RCFLAGS) $(RES)$/scappi.res                    >>$@
.ENDIF

.IF "$(GUI)" == "OS2"

$(MISC)$/$(SHL1TARGET).def:  makefile.mk
    @echo ------------------------------
    @echo Making: $@
    @echo LIBRARY     $(SHL1TARGET8) INITINSTANCE TERMINSTANCE     >$@
    @echo DESCRIPTION 'SCALC3 DLL'                                 >>$@
    @echo DATA        MULTIPLE                                     >>$@
    @echo EXPORTS                                                   >>$@
    @echo   _CreateScDocShellDll                                >>$@
    @echo   _CreateObjScDocShellDll                             >>$@
    @echo   _InitScDll                                              >>$@
    @echo   _DeInitScDll                                            >>$@
    @echo   _component_getImplementationEnvironment                 >>$@
    @echo   _component_writeInfo                                    >>$@
    @echo   _component_getFactory                                   >>$@
.ENDIF
