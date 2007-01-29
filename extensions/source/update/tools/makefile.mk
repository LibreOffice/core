#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: rt $ $Date: 2007-01-29 14:36:01 $
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
PRJNAME = extensions
TARGET = ztool
LIBTARGET = NO
TARGETTYPE = CUI
ENABLE_EXCEPTIONS=TRUE

.INCLUDE: settings.mk

OBJFILES = \
    $(OBJ)$/ztool.obj

APP1TARGET = ztool
APP1OBJS = $(OBJ)$/ztool.obj
APP1STDLIBS = $(TOOLSLIB)

#APP1TARGET = ztool
#APP1OBJS = $(OBJ)$/ztool.obj
#.IF "$(GUI)" == "UNX"
#APP1STDLIBS = $(TOOLSLIB)
#.ELSE
#APP1LIBS = $(LB)$/itools.lib
#.ENDIF

#APP2TARGET = urltest
#APP2OBJS = $(OBJ)$/urltest.obj
#.IF "$(GUI)" == "UNX"
#APP2STDLIBS = $(TOOLSLIB) $(VOSLIB) $(SALLIB) $(CPPULIB) $(CPPUHELPERLIB)
#.ELSE
#APP2STDLIBS = $(LB)$/itools.lib ivos.lib isal.lib icppu.lib icppuhelper.lib
#.ENDIF

# APP3TARGET = tldem
# APP3OBJS = $(OBJ)$/tldem.obj
# .IF "$(GUI)" == "UNX"
# APP3STDLIBS = $(TOOLSLIB)
# .ELSE
# APP3STDLIBS = $(LB)$/itools.lib
# .ENDIF

.INCLUDE: target.mk
