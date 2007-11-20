#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.13 $
#
#   last change: $Author: ihi $ $Date: 2007-11-20 19:32:03 $
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
PRJ=..$/..$/..

PRJNAME=sal
TARGET=qa_osl_process
# this is removed at the moment because we need some enhancements
# TESTDIR=TRUE

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

.IF "$(GUI)" != "OS2"

CFLAGS+= $(LFS_CFLAGS)
CXXFLAGS+= $(LFS_CFLAGS)

# BEGIN ----------------------------------------------------------------
# auto generated Target:testjob by codegen.pl 

.IF "$(GUI)" == "WNT"
    CFLAGS+=/Ob1
.ENDIF

SHL1OBJS=  \
    $(SLO)$/osl_Thread.obj

SHL1TARGET= osl_Thread
SHL1STDLIBS=\
   $(SALLIB) 
.IF "$(GUI)" == "WNT" || "$(GUI)" == "OS2"
SHL1STDLIBS+=	$(SOLARLIBDIR)$/cppunit.lib
.ENDIF
.IF "$(GUI)" == "UNX"
SHL1STDLIBS+=$(SOLARLIBDIR)$/libcppunit$(DLLPOSTFIX).a
# .IF "$(OS)" == "SOLARIS"
# SHL1STDLIBS += -lrt -laio
# .ENDIF
.ENDIF

SHL1IMPLIB= i$(SHL1TARGET)

# SHL1DEF=    $(MISC)$/$(SHL1TARGET).def

DEF1NAME    =$(SHL1TARGET)

# DEF1EXPORTFILE= export.exp
SHL1VERSIONMAP = $(PRJ)$/qa$/export.map

.ENDIF # "$(GUI)" != "OS2"

# END ------------------------------------------------------------------

#.IF "$(GUI)" == "WNT"

SHL2OBJS=$(SLO)$/osl_process.obj
SHL2TARGET=osl_process
SHL2STDLIBS=$(SALLIB) 

.IF "$(GUI)" == "WNT" || "$(GUI)" == "OS2"
SHL2STDLIBS+=$(SOLARLIBDIR)$/cppunit.lib
.ENDIF
.IF "$(GUI)" == "UNX"
SHL2STDLIBS+=$(SOLARLIBDIR)$/libcppunit$(DLLPOSTFIX).a
.ENDIF

SHL2IMPLIB=i$(SHL2TARGET)
SHL2DEF=$(MISC)$/$(SHL2TARGET).def
DEF2NAME=$(SHL2TARGET)
DEF2EXPORTFILE=export.exp

# END ------------------------------------------------------------------

OBJ3FILES=$(OBJ)$/osl_process_child.obj 
APP3TARGET=osl_process_child
APP3OBJS=$(OBJ3FILES)

# .IF "$(GUI)" == "UNX"
# APP3STDLIBS=$(LB)$/libsal.so
# .ENDIF
# .IF "$(GUI)" == "WNT"
# APP3STDLIBS=$(KERNEL32LIB) $(LB)$/isal.lib
# .ENDIF
APP3STDLIBS=$(SALLIB)

#.ENDIF # "$(GUI)" == "WNT"

#------------------------------- All object files -------------------------------
# do this here, so we get right dependencies

.IF "$(GUI)" == "OS2"

SLOFILES=$(SHL2OBJS)

.ELSE

SLOFILES=$(SHL1OBJS) $(SHL2OBJS)

.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE : _cppunit.mk
