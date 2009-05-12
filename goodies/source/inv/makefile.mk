#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.21 $
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

PRJ=..$/..

PRJNAME=goodies
TARGET=invader
TARGETTYPE=GUI

EXTERNAL_WARNINGS_NOT_ERRORS := TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

.IF "$(TEST)" != ""
CDEFS+=-DTEST
OBJFILES=	\
        $(OBJ)$/invader.obj \
        $(OBJ)$/invader1.obj \
        $(OBJ)$/shapes.obj \
        $(OBJ)$/expl.obj \
        $(OBJ)$/score.obj \
        $(OBJ)$/monst.obj
.ELSE

SLOFILES= \
        $(SLO)$/invader.obj \
        $(SLO)$/invader1.obj \
        $(SLO)$/shapes.obj \
        $(SLO)$/expl.obj \
        $(SLO)$/score.obj \
        $(SLO)$/monst.obj
.ENDIF

SRS1NAME=$(TARGET)
SRC1FILES=gegner.src strings.src

RESLIB1IMAGES= $(PRJ)$/res
RESLIB1NAME=tfu
RESLIB1SRSFILES=$(SRS)$/invader.srs

.IF "$(TEST)"!=""
APP1TARGET= $(TARGET)
APP1OBJS= $(OBJFILES)
APP1STDLIBS=	$(CPPULIB)			\
                $(CPPUHELPERLIB)	\
                $(COMPHELPERLIB)	\
                $(TOOLSLIB) 		\
                $(SALLIB)			\
                $(VOSLIB)			\
                $(SOTLIB)			\
                $(VCLLIB)

.ELSE

SHL1TARGET= tfu$(DLLPOSTFIX)
SHL1IMPLIB= itfu
SHL1STDLIBS=\
            $(VCLLIB)     \
            $(TOOLSLIB)  \
            $(SALLIB)

.IF "$(GUI)"=="WNT"
SHL1STDLIBS+=\
            $(COMDLG32LIB) \
            $(ADVAPI32LIB)
.ENDIF

SHL1OBJS=$(SLOFILES)

SHL1DEF=$(MISC)$/$(SHL1TARGET).def

.ENDIF
# --- Targets -------------------------------------------------------


.INCLUDE :	target.mk

ALLTAR: \
    $(BIN)$/applicat.rdb

$(BIN)$/applicat.rdb : makefile.mk $(SOLARBINDIR)$/types.rdb
    rm -f $@
    $(GNUCOPY) $(SOLARBINDIR)$/types.rdb $@
    cd $(BIN) && \
        $(REGCOMP) -register -r applicat.rdb \
            -c i18npool.uno$(DLLPOST)

# --- Def-File ---

.IF "$(GUI)"=="WNT"

$(MISC)$/$(SHL1TARGET).def: makefile.mk
    @echo -------------------------------------------
    @echo DEF-File erstellen $@
    @echo LIBRARY     $(DLLNAME)                    >$@
.IF "$(COM)"!="GCC"
    @echo DESCRIPTION 'Star Cube DLL'               >>$@
    @echo DATA                READ WRITE NONSHARED  >>$@
.ENDIF
    @echo EXPORTS                                   >>$@
    @echo     StartInvader    @22                    >>$@

.ENDIF

.IF "$(GUI)"=="OS2"

$(MISC)$/$(SHL1TARGET).def: makefile.mk
    @+echo -------------------------------------------
    @+echo DEF-File erstellen $@
        @+echo LIBRARY     $(SHL1TARGET) INITINSTANCE TERMINSTANCE   >$@
        @+echo DESCRIPTION 'Star Cube DLL'               >>$@
        @+echo DATA                MULTIPLE  >>$@
        @+echo EXPORTS                                   >>$@
        @+echo     _StartInvader    @22                    >>$@

.ENDIF

