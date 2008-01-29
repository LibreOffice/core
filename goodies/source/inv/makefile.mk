#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.17 $
#
#   last change: $Author: rt $ $Date: 2008-01-29 14:42:39 $
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

PRJ=..$/..

PRJNAME=goodies
TARGET=invader
TARGETTYPE=GUI

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

SHL1TARGET= tfu$(UPD)$(DLLPOSTFIX)
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
        regcomp -register -r applicat.rdb \
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
        @+echo LIBRARY     $(DLLNAME) INITINSTANCE TERMINSTANCE   >$@
        @+echo DESCRIPTION 'Star Cube DLL'               >>$@
        @+echo DATA                MULTIPLE  >>$@
        @+echo EXPORTS                                   >>$@
        @+echo     _StartInvader    @22                    >>$@

.ENDIF

