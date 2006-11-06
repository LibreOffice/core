#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.11 $
#
#   last change: $Author: kz $ $Date: 2006-11-06 14:55:20 $
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

PRJ = ..$/..$/..

PRJNAME = desktop
TARGET = deployment_misc
ENABLE_EXCEPTIONS = TRUE
LIBTARGET = NO

.INCLUDE : settings.mk

.IF "$(SYSTEM_DB)" == "YES"
CFLAGS+=-DSYSTEM_DB -I$(DB_INCLUDES)
.ENDIF

SRS1NAME = $(TARGET)
SRC1FILES = \
    dp_misc.src

LIB1TARGET = $(SLB)$/$(TARGET).lib
LIB1OBJFILES = \
        $(SLO)$/dp_misc.obj \
        $(SLO)$/dp_resource.obj \
        $(SLO)$/dp_interact.obj \
        $(SLO)$/dp_ucb.obj \
        $(SLO)$/dp_xml.obj \
        $(SLO)$/dp_log.obj \
        $(SLO)$/dp_persmap.obj \
        $(SLO)$/dp_services.obj       \
        $(SLO)$/db.obj \
        $(SLO)$/dp_version.obj

SLOFILES = $(LIB1OBJFILES)

OBJFILES = $(OBJ)$/dp_version.obj

.INCLUDE : ..$/target.pmk
.INCLUDE : target.mk

