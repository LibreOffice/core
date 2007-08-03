#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: hr $ $Date: 2007-08-03 10:17:02 $
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
PRJINC=

PRJNAME=testshl2
TARGET=onefunc
TARGETTYPE=CUI

ENABLE_EXCEPTIONS=TRUE
#USE_LDUMP2=TRUE
#LDUMP2=LDUMP3

# --- Settings -----------------------------------------------------
.INCLUDE :  settings.mk

# ------------------------------------------------------------------
#--------------------------------- Objectfiles ---------------------------------
OBJFILES=$(OBJ)$/onefuncstarter.obj 
SLOFILES=$(SLO)$/onefunc.obj

# -------------------------------------------------------------------------------

LIB1TARGET=   $(LB)$/$(TARGET)_libfile.lib
LIB1OBJFILES= $(SLOFILES)

.IF "$(GUI)" == "UNX"
LIB1ARCHIV=$(LB)$/lib$(TARGET)$(DLLPOSTFIX).a
.ENDIF

# ------------------------------------------------------------------
SHL2OBJS = $(SLOFILES)

SHL2TARGET= $(TARGET)_DLL
SHL2STDLIBS+=\
            $(SALLIB)

SHL2DEPN=
SHL2IMPLIB= i$(SHL2TARGET)
SHL2DEF=    $(MISC)$/$(SHL2TARGET).def

DEF2NAME    =$(SHL2TARGET)
DEF2EXPORTFILE= export.exp

#----------------------------- prog with *.lib file -----------------------------

APP1TARGET= $(TARGET)starter
APP1OBJS=$(OBJFILES)

APP1STDLIBS=$(SALLIB)

APP1DEPN= $(APP1OBJS) $(LIB1TARGET)
APP1LIBS= $(LIB1TARGET)

#-------------------------- executable with LIBARCHIV --------------------------

APP2TARGET= $(TARGET)starter2
APP2OBJS=   $(OBJFILES)

APP2STDLIBS=$(SALLIB)
.IF "$(GUI)" == "UNX"
APP2STDLIBS+=$(LB)$/lib$(TARGET)$(DLLPOSTFIX).a
.ENDIF
.IF "$(GUI)" == "WNT"
APP2STDLIBS+=$(LIB1TARGET)
.ENDIF

APP2DEPN= $(APP1OBJS) $(LIB1TARGET)
APP2LIBS= 

#----------------------- executable with dynamic library -----------------------

APP3TARGET= $(TARGET)starter3
APP3OBJS=   $(OBJFILES)

APP3STDLIBS=$(SALLIB)

.IF "$(GUI)" == "WNT"
APP3STDLIBS+=i$(SHL2TARGET).lib
.ENDIF
.IF "$(GUI)" == "UNX"
APP3STDLIBS+=-l$(SHL2TARGET)
.ENDIF

APP3DEPN= $(APP1OBJS) $(LIB1TARGET)
APP3LIBS= 
# APP3DEF=

#----------------------- executable without any other files -----------------------

APP4TARGET= $(TARGET)starter4
APP4OBJS=   $(OBJFILES)

APP4STDLIBS=$(SALLIB)

APP4DEPN= $(APP1OBJS)
APP4LIBS= 
# APP3DEF=
# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE : _cppunit.mk
