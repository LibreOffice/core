#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: pb $ $Date: 2000-11-03 14:52:34 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#
#*************************************************************************

PRJ=..$/..

PRJNAME=goodies
TARGET=invader

AUTOSEG=true

# --- Settings -----------------------------------------------------

.INCLUDE :  svpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sv.mk

# --- Files --------------------------------------------------------

.IF "$(TEST)" != ""
ENVCFLAGS=-DTEST
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

SRCFILES=gegner.src strings.src

RESLIB1NAME=tfu
RESLIB1SRSFILES=$(SRS)$/invader.srs

.IF "$(TEST)"!=""
APP1TARGET= $(TARGET)
APP1OBJS= $(OBJ)$/invader.obj
APP1STDLIBS= $(TOOLSLIB) $(SVLIB)
APP1LIBS=$(LIBPRE) invader.lib
APP1STACK= 64000
APP1DEPN= $(LB)$/invader.lib

.ELSE

SHL1TARGET= tfu$(UPD)$(DLLPOSTFIX)
SHL1IMPLIB= itfu
SHL1STDLIBS=\
            $(SVLIB)     \
            $(TOOLSLIB)

.IF "$(GUI)"=="WNT"
SHL1STDLIBS+=\
            comdlg32.lib \
            advapi32.lib
.ENDIF

.IF "$(GUI)"!="WNT"
SHL1LIBS=   $(LIB1TARGET)
.ENDIF

.IF "$(GUI)"=="WNT"
SHL1OBJS=$(SLOFILES)
.ELSE
SHL1LIBS=$(SLB)$/$(TARGET).lib
.ENDIF

SHL1DEF=$(MISC)$/$(SHL1TARGET).def

.ENDIF
# --- Targets -------------------------------------------------------


.INCLUDE :  target.mk

.IF "$(GUI)"=="WIN"

$(MISC)$/$(APP1TARGET).def : makefile.mk
    echo  NAME			svdiff 								>$@
    echo  DESCRIPTION	'Star Invader'		   >>$@
    echo  EXETYPE		WINDOWS 						   >>$@
    echo  STUB			'winSTUB.EXE'                      >>$@
    echo  PROTMODE										   >>$@
    echo  CODE			LOADONCALL MOVEABLE DISCARDABLE    >>$@
    echo  DATA			PRELOAD MOVEABLE MULTIPLE		   >>$@
    echo  HEAPSIZE		8192							   >>$@
    echo  STACKSIZE 	32000							   >>$@

.ENDIF

# --- Def-File ---

.IF "$(GUI)"=="WNT"

$(MISC)$/$(SHL1TARGET).def: makefile.mk $(MISC)$/$(SHL1TARGET).flt
    @+echo -------------------------------------------
    @+echo DEF-File erstellen $@
        @+echo LIBRARY     $(DLLNAME)                    >$@
        @+echo DESCRIPTION 'Star Cube DLL'               >>$@
        @+echo DATA                READ WRITE NONSHARED  >>$@
        @+echo EXPORTS                                   >>$@
        @+echo     StartInvader    @22                    >>$@

.ENDIF

.IF "$(GUI)"=="OS2"

$(MISC)$/$(SHL1TARGET).def:\
    makefile.mk \
    $(MISC)$/$(SHL1TARGET).flt
    @echo -------------------------------------------
    @echo DEF-File erstellen
.IF "$(COM)"!="WTC"
    @echo LIBRARY     $(DLLNAME) INITINSTANCE                        >$@
    @echo DESCRIPTION 'StarView Filter DLL'                         >>$@
    @echo PROTMODE                                                  >>$@
    @echo CODE        LOADONCALL                                    >>$@
    @echo DATA        PRELOAD MULTIPLE NONSHARED                    >>$@
    @echo EXPORTS                                                   >>$@
.IF "$(COM)"=="ICC"
    @echo    StartInvader		                                    >>$@
.ELSE
    @echo    _StartInvader		                                    >>$@
.ENDIF
.ELSE
        @echo option DESCRIPTION 'StarWars Dll'		                    >$@
        @echo name $(BIN)$/$(SHL1TARGET).dll                             >>$@
    @echo StartInvader_ @2 				>>temp.def
    @gawk -f s:\util\exp.awk temp.def				>>$@
    del temp.def

.ENDIF
.ENDIF #os2

$(MISC)$/$(SHL1TARGET).flt:
    @+echo $(MISC)
    @echo $(@)
    @+echo $(@)
    @echo Imp		>$(@)

