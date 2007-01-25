#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.6 $
#
#   last change: $Author: obo $ $Date: 2007-01-25 13:57:59 $
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

RTLLIB=irtl.lib

PRJ=..

PRJPCH=

PRJNAME=vos
TARGET=vos
USE_LDUMP2=TRUE
#USE_DEFFILE=TRUE

.IF "$(GUI)"=="UNX"
TARGETTYPE=CUI
.ENDIF

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  ..$/version.mk

# --- Files --------------------------------------------------------

#.IF "$(UPDATER)"=="YES"
#LIB1TARGET=$(LB)$/a$(TARGET).lib
#LIB1ARCHIV=$(LB)$/lib$(TARGET)$(VOS_MAJOR)$(DLLPOSTFIX).a
#LIB1FILES=$(LB)$/cpp$(TARGET).lib
#.ENDIF

SHL1TARGET=$(VOS_TARGET)$(VOS_MAJOR)$(COMID)
SHL1IMPLIB=i$(TARGET)

.IF "$(GUI)"=="WNT"
SHL1STDLIBS=wsock32.lib $(SALLIB)
.ELSE
SHL1STDLIBS=$(SALLIB)
.ENDIF

.IF "$(GUI)"=="WIN"
SHL1STDLIBS+=	\
                winsock.lib
.ENDIF


SHL1LIBS=    $(SLB)$/cpp$(TARGET).lib
.IF "$(GUI)" != "UNX"
.IF "$(GUI)" != "MAC"
SHL1OBJS=    \
    $(SLO)$/object.obj
.ENDIF
.ENDIF

SHL1DEPN=
SHL1DEF=    $(MISC)$/$(SHL1TARGET).def

DEF1NAME    =$(SHL1TARGET)
DEF1DEPN    =$(MISC)$/$(SHL1TARGET).flt
DEFLIB1NAME =cppvos

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

$(MISC)$/$(SHL1TARGET).flt:
    @echo ------------------------------
    @echo Making: $@
    @echo WEP > $@
    @echo LIBMAIN >> $@
    @echo LibMain >> $@
    @echo _alloc >> $@
    @echo alloc >> $@
    @echo _CT >> $@
    @echo _TI2 >> $@
    @echo _TI1 >> $@
    @echo exception::exception >> $@
    @echo @std@ >> $@
    @echo __>>$@

