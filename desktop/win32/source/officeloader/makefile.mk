#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.4 $
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

PRJ=..$/..$/..

PRJNAME=desktop
TARGET=officeloader
LIBTARGET=NO
#TARGETTYPE=GUI

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

OBJFILES = \
        $(OBJ)$/officeloader.obj

#APP1TARGET=so$/officeloader
#APP1NOSAL=TRUE
#APP1LINKRES=$(MISC)$/$(TARGET).res
#APP1ICON=$(SOLARRESDIR)$/icons/001_star_main.ico
#APP1OBJS = \
#		$(OBJ)$/officeloader.obj

#APP2TARGET=officeloader
#APP2NOSAL=TRUE
#APP2LINKRES=$(MISC)$/$(TARGET).res
#APP2ICON=$(SOLARRESDIR)$/icons/ooo_gulls.ico
#APP2OBJS = \
#		$(OBJ)$/officeloader.obj


# --- Targets ------------------------------------------------------


.INCLUDE :  target.mk

