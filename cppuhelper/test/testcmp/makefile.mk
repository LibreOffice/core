#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: kz $ $Date: 2007-05-09 13:28:08 $
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

PRJ               := ..$/..
PRJNAME           := cppuhelper
TARGET            := TestComponent.uno
ENABLE_EXCEPTIONS := TRUE


.INCLUDE: settings.mk


SHL1TARGET     := $(TARGET)
SHL1OBJS       := $(SLO)$/TestComponent.obj
SHL1STDLIBS    := $(CPPUHELPERLIB) $(CPPULIB) $(SALHELPERLIB) $(SALLIB) 
SHL1IMPLIB     := i$(SHL1TARGET)
.IF "$(GUI)"=="WNT"
SHL1DEF        := TestComponent.uno.def
.ELSE
DEF1NAME       := $(SHL1TARGET)
.ENDIF


.INCLUDE: target.mk

