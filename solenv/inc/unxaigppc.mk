#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
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

# mk file for Unix AIX PowerPC using GCC, please make generic modifications to unxlng.mk

LIBSALCPPRT*=-Wl,-bnogc -lsalcpprt -Wl,-bgc

.INCLUDE : unxlngppc.mk

CDEFS+=-D_THREAD_SAFE

.INCLUDE : productversion.mk

COLON=":"
URELIBDIRS=$(subst,$(SPACECHAR),$(COLON) $(foreach,i,{$(PRODUCTLIST)} /opt/$i$(PRODUCTVERSIONSHORT)/basis-link/ure-link/lib))
UREBINDIRS=$(subst,$(SPACECHAR),$(COLON) $(foreach,i,{$(PRODUCTLIST)} /opt/$i$(PRODUCTVERSIONSHORT)/basis-link/ure-link/bin))
BASISPROGRAMDIRS=$(subst,$(SPACECHAR),$(COLON) $(foreach,i,{$(PRODUCTLIST)} /opt/$i$(PRODUCTVERSIONSHORT)/basis-link/program))
BRANDPROGRAMDIRS=$(subst,$(SPACECHAR),$(COLON) $(foreach,i,{$(PRODUCTLIST)} /opt/$i$(PRODUCTVERSIONSHORT)/program))

LINKFLAGSRUNPATH_URELIB:=-Wl,-blibpath:$(URELIBDIRS):/usr/lib:/lib
LINKFLAGSRUNPATH_UREBIN:=-Wl,-blibpath:$(URELIBDIRS):$(UREBINDIRS):/usr/lib:/lib
LINKFLAGSRUNPATH_OOO:=-Wl,-blibpath:$(BASISPROGRAMDIRS):$(URELIBDIRS):/usr/lib:/lib
LINKFLAGSRUNPATH_SDK:=-Wl,-blibpath:$(URELIBDIRS):/usr/lib:/lib
LINKFLAGSRUNPATH_BRAND:=-Wl,-blibpath:$(BRANDPROGRAMDIRS):$(BASISPROGRAMDIRS):$(URELIBDIRS):/usr/lib:/lib
LINKFLAGSRUNPATH_BOXT:=-Wl,-blibpath:$(BASISPROGRAMDIRS):/usr/lib:/lib
LINKFLAGS:=-Wl,-brtl -Wl,-bnolibpath

LINKFLAGSAPPGUI:=
LINKFLAGSAPPCUI:=

LINKVERSIONMAPFLAG:=

SONAME_SWITCH:=

STDLIBGUIMT:=-ldl -lpthread -lm
STDLIBCUIMT:=-ldl -lpthread -lm
STDSHLGUIMT:=-ldl -lpthread -lm
STDSHLCUIMT:=-ldl -lpthread -lm
X11LINK_DYNAMIC:=-lXext -lX11


CFLAGSDEBUG:=

OOO_LIBRARY_PATH_VAR=LIBPATH
