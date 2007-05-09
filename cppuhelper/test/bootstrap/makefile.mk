#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: kz $ $Date: 2007-05-09 13:27:04 $
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

PRJ     := ..$/..
PRJNAME := cppuhelper
TARGET  := bootstrap.test


ENABLE_EXCEPTIONS  := TRUE
NO_BSYMBOLIC       := TRUE
NO_SHL_DESCRIPTION := TRUE
USE_DEFFILE        := TRUE


.INCLUDE :  settings.mk


.IF "$(GUI)"=="UNX" || "$(GUI)"=="MAC"
PURPENVHELPERLIB := -luno_purpenvhelper$(COMID)

.ELSE
PURPENVHELPERLIB := $(LIBPRE) ipurpenvhelper$(UDK_MAJOR)$(COMID).lib

.ENDIF


APP1TARGET  := $(TARGET)
APP1OBJS    := $(OBJ)$/bootstrap.test.obj
APP1STDLIBS := $(CPPUHELPERLIB) $(CPPULIB) $(SALLIB) 

SHL1TARGET      := testenv_uno_uno
SHL1IMPLIB      := i$(SHL1TARGET)
SHL1OBJS        := $(SLO)$/TestEnv.obj
SHL1STDLIBS     := $(PURPENVHELPERLIB) $(SALHELPERLIB) $(SALLIB)
SHL1DEF         := TestEnv.def


.INCLUDE :  target.mk
