#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.9 $
#
#   last change: $Author: vg $ $Date: 2007-03-26 13:45:28 $
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

PRJ = ..
PRJNAME = tl
TARGET = tldem
LIBTARGET = NO
TARGETTYPE = CUI
ENABLE_EXCEPTIONS=TRUE

.INCLUDE: settings.mk

OBJFILES = \
    $(OBJ)$/solar.obj \
    $(OBJ)$/urltest.obj \
    $(OBJ)$/inetmimetest.obj
#	$(OBJ)$/demostor.obj \
#	$(OBJ)$/fstest.obj \
#	$(OBJ)$/tldem.obj \

APP1TARGET = solar
APP1OBJS = $(OBJ)$/solar.obj
.IF "$(GUI)" == "UNX"
APP1STDLIBS = $(TOOLSLIB)
.ELSE
APP1LIBS = $(LB)$/itools.lib
.ENDIF

APP2TARGET = urltest
APP2OBJS = $(OBJ)$/urltest.obj
.IF "$(GUI)" == "UNX"
APP2STDLIBS = $(TOOLSLIB) $(VOSLIB) $(SALLIB) $(CPPULIB) $(CPPUHELPERLIB)
.ELSE
APP2STDLIBS = $(LB)$/itools.lib $(VOSLIB) $(SALLIB) $(CPPULIB) $(CPPUHELPERLIB)
.ENDIF

APP3TARGET = inetmimetest
APP3OBJS = $(OBJ)$/inetmimetest.obj
APP3STDLIBS = $(SALLIB) $(TOOLSLIB)

# APP3TARGET = tldem
# APP3OBJS = $(OBJ)$/tldem.obj
# .IF "$(GUI)" == "UNX"
# APP3STDLIBS = $(TOOLSLIB)
# .ELSE
# APP3STDLIBS = $(LB)$/itools.lib
# .ENDIF

# APP4TARGET = demostor
# APP4OBJS = $(OBJ)$/demostor.obj
# .IF "$(GUI)" == "UNX"
# APP4STDLIBS = $(TOOLSLIB) $(VOSLIB) $(SALLIB)
# .ELSE
# APP4STDLIBS = $(LB)$/itools.lib $(VOSLIB) $(SALLIB)
# .ENDIF

# APP5TARGET = fstest
# APP5OBJS = $(OBJ)$/fstest.obj
# .IF "$(GUI)" == "UNX"
# APP5STDLIBS = $(TOOLSLIB) $(VOSLIB) $(SALLIB)
# .ELSE
# APP5STDLIBS = $(LB)$/itools.lib $(VOSLIB) $(SALLIB)
# .ENDIF

.INCLUDE: target.mk
