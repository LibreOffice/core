#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.3 $
#
#   last change: $Author: rt $ $Date: 2005-09-08 17:00:12 $
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

PRJ := ..
PRJNAME := salhelper
TARGET := qa

ENABLE_EXCEPTIONS := TRUE

.INCLUDE: settings.mk

SHL1TARGET = $(TARGET)
SHL1OBJS = $(SLO)$/test_api.obj
SHL1STDLIBS = $(CPPUNITLIB) $(SALLIB) $(SALHELPERLIB)
SHL1IMPLIB = i$(SHL1TARGET)
SHL1VERSIONMAP = version.map
DEF1NAME = $(SHL1TARGET)

SLOFILES = $(SHL1OBJS)

.INCLUDE: target.mk

ALLTAR: test

test .PHONY: $(SHL1TARGETN)
    + cd $(SHL1TARGETN:d) && testshl2 $(SHL1TARGETN:f)
