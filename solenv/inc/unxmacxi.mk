#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: unxmacxi.mk,v $
#
# $Revision: 1.6 $
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

#
# Mac OS X/Intel specific defines
#

PROCESSOR_DEFINES=-DX86

DLLPOSTFIX=mxi

# special for SO build environment
.IF "$(SYSBASE)"!=""
.IF "$(EXTRA_CFLAGS)"!=""
CPP:=gcc -E $(EXTRA_CFLAGS)
CXXCPP*:=g++ -E $(EXTRA_CFLAGS)
.EXPORT : CPP CXXCPP
.ENDIF # "$(EXTRA_CFLAGS)"!=""
.ENDIF # "$(SYSBASE)"!=""

# flags to enable build with symbols; required by crashdump feature
.IF "$(ENABLE_SYMBOLS)"=="SMALL"
CFLAGSENABLESYMBOLS=-g1
.ELSE
CFLAGSENABLESYMBOLS=-g
.ENDIF

.IF "$(SNOW_LEOPARD_10_4)"!=""
JAVACOMPILER+=-target 1.5
DYLD_INSERT_LIBRARIES=/usr/lib/libsqlite3.dylib
.EXPORT: DYLD_INSERT_LIBRARIES
.ENDIF # "$(SNOW_LEOPARD_10_4)"!=""

# Include generic Mac OS X makefile
.INCLUDE : unxmacx.mk
