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
# $Revision: 1.8 $
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

PRJ=..$/..

PRJNAME=rsc
TARGETTYPE=CUI
TARGET=rsc
LIBTARGET=NO

# --- Settings -----------------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------------

OBJFILES=   $(OBJ)$/gui.obj          \
            $(OBJ)$/start.obj

APP1TARGET= rsc
APP1STDLIBS=$(TOOLSLIB) $(I18NISOLANGLIB) $(VOSLIB) $(SALLIB) # $(RTLLIB)
APP1LIBS=   $(LB)$/rsctoo.lib
APP1OBJS=   $(OBJ)$/start.obj
.IF "$(GUI)" != "OS2"
# why not this way?
APP1STACK=64000
#APP1STACK=32768
.ENDIF

APP2TARGET= rsc2
.IF "$(OS)"=="SCO"
# SCO hat Probleme mit fork/exec und einigen shared libraries. 
# rsc2 muss daher statisch gelinkt werden
APP2STDLIBS=$(STATIC) -latools $(BPICONVLIB) $(VOSLIB) $(OSLLIB) $(RTLLIB) $(DYNAMIC)
.ELSE
APP2STDLIBS=$(TOOLSLIB) $(I18NISOLANGLIB) $(VOSLIB) $(SALLIB) #RTLLIB)
.ENDIF
APP2LIBS=   $(LB)$/rsctoo.lib \
            $(LB)$/rscres.lib \
            $(LB)$/rscpar.lib \
            $(LB)$/rscrsc.lib \
            $(LB)$/rscmis.lib
APP2OBJS=   $(OBJ)$/gui.obj
.IF "$(GUI)" != "OS2"
# why not this way?
APP2STACK=64000
#APP2STACK=32768
.ENDIF

# --- Targets ------------------------------------------------------------

.INCLUDE :  target.mk

