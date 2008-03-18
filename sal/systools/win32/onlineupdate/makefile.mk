#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.6 $
#
#   last change: $Author: vg $ $Date: 2008-03-18 13:21:22 $
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
TARGET=onlinecheck

USE_DEFFILE=TRUE
LIBTARGET=NO

# --- Settings ----------------------------------

.INCLUDE : settings.mk

CFLAGS+= $(LFS_CFLAGS)
CXXFLAGS+= $(LFS_CFLAGS)

# --- Targets ----------------------------------

.IF "$(GUI)"=="WNT"

.IF "$(COMEX)"=="9"
.IF "$(PSDK_HOME)"!=""
# Since the 02/2003 PSDK the "new" linker is needed here.
LINK=$(WRAPCMD) "$(PSDK_HOME)$/Bin$/Win64$/LINK.EXE"
.ENDIF
.ENDIF

SLOFILES=\
        $(SLO)$/onlinecheck.obj
        

SHL1TARGET=$(TARGET)
SHL1IMPLIB=$(SHL1TARGET)
SHL1DEF=$(MISC)/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=$(SHL1TARGET).dxp

DEF1DEPN=\
        $(DEF1EXPORTFILE)\
        makefile.mk

SHL1OBJS=$(SLOFILES)

#No default libraries
#STDSHL=

SHL1STDLIBS=\
        $(UNICOWSLIB)\
        $(KERNEL32LIB)\
        $(LIBCMT)\
        $(WININETLIB)

        
.ENDIF

.INCLUDE : target.mk

