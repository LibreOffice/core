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
# $Revision: 1.9 $
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
PRJNAME=extensions
TARGET=msidl

# --- Settings ----------------------------------

.INCLUDE : settings.mk

.IF "$(GUI)" == "WNT" && "$(DISABLE_ACTIVEX)"==""

# --- Targets ----------------------------------

ALL : $(MISC)$/activex_component

MTL=midl.exe
MTL_PROJ=

SOURCE=so_activex.idl

MTL_SWITCHES=-tlb $(MISC)$/so_activex.tlb -h $(MISC)$/so_activex.h -iid $(MISC)$/so_activex_i.c -dlldata $(MISC)$/so_activex_dll.c -proxy $(MISC)$/so_activex_p.c -Oicf

.IF "$(COM)"=="GCC"
MTL_SWITCHES+=-cpp_cmd gcc -cpp_opt "-x c -E -D__stdcall= -D_stdcall= -nostdinc -I $(shell @cygpath -w $(PSDK_HOME)/include)"

$(MISC)$/activex_component : $(SOURCE) 
    $(MTL) $(MTL_SWITCHES) -I $(shell @cygpath -w $(PSDK_HOME)/include|sed 's#\\#\\\\#g') $(SOURCE) 
.ELSE
$(MISC)$/activex_component : $(SOURCE) 
    $(WRAPCMD) $(MTL) $(MTL_SWITCHES) $(INCLUDE) $(SOURCE) && $(TOUCH) $@
.ENDIF

.ENDIF

.INCLUDE : target.mk

