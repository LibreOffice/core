#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.6 $
#
#   last change: $Author: rt $ $Date: 2008-01-29 14:41:40 $
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

PRJNAME=bridges
TARGET=inter
LIBTARGET=NO
TARGETTYPE=CUI
ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

UNOUCRDEP=$(SOLARBINDIR)$/udkapi.rdb
UNOUCRRDB=$(SOLARBINDIR)$/udkapi.rdb

# output directory (one dir for each project)
UNOUCROUT=$(OUT)$/inc$/$(TARGET)

# adding to inludeoath
INCPRE+=$(UNOUCROUT)

UNOTYPESTYPES := \
        com.sun.star.lang.IllegalArgumentException \
        com.sun.star.uno.DeploymentException

SLOFILES=$(SLO)$/starter.obj $(SLO)$/thrower.obj

SHL1TARGET=starter
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
SHL1IMPLIB=i$(SHL1TARGET)
SHL1OBJS=$(SLO)$/starter.obj
DEF1NAME=$(SHL1TARGET)
SHL1STDLIBS+= $(CPPULIB) $(SALLIB)
SHL1VERSIONMAP=$(SHL1TARGET).map

SHL2TARGET=thrower
SHL2DEF=$(MISC)$/$(SHL2TARGET).def
SHL2IMPLIB=i$(SHL2TARGET)
SHL2OBJS=$(SLO)$/thrower.obj
DEF2NAME=$(SHL2TARGET)
SHL2STDLIBS+= $(CPPULIB) $(SALLIB)
SHL2VERSIONMAP=$(SHL2TARGET).map

OBJFILES=$(OBJ)$/inter.obj
APP1TARGET=inter
APP1OBJS=$(OBJ)$/inter.obj
APP1STDLIBS+=\
        $(SALLIB)

.INCLUDE :  target.mk

