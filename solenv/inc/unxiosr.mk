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

# --- Unix Environment for iOS cross-compilation -------------

CFLAGS=-c $(EXTRA_CFLAGS)

LINKC*=$(CC)
LINK*=$(CXX)

STDLIBCUIMT=-framework CoreFoundation
STDLIBGUIMT=-framework CoreFoundation
STDSHLCUIMT=-framework CoreFoundation
STDSHLGUIMT=-framework CoreFoundation

LIBMGR*=ar
LIBFLAGS=-r

PROCESSOR_DEFINES=-DARM32

DLLPRE=lib
DLLPOST=.a

# flags to enable build with symbols
CFLAGSENABLESYMBOLS=-g

# Flag for including debugging information in object files
CFLAGSDEBUG=-g
CFLAGSDBGUTIL=

# Flag to specify output file to compiler/linker
CFLAGSOUTOBJ=-o

# # Include generic Mac OS X makefile
# .INCLUDE : unxmacx.mk

# --- general *ix settings ---
HC=hc
HCFLAGS=
PATH_SEPERATOR*=:
CDEFS+=-D__DMAKE

CDEFS+=-DUNIX

YACC*=yacc
YACCFLAGS*=-d -t

EXECPOST=
SCPPOST=.ins
DLLDEST=$(LB)

OOO_LIBRARY_PATH_VAR = DYLD_LIBRARY_PATH
