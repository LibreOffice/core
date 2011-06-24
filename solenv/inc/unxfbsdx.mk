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

#
# FreBSD/amd64 specific defines
#

CDEFS+=-DX86_64

# architecture dependent flags for the C and C++ compiler that can be changed by
# exporting the variable ARCH_FLAGS="..." in the shell, which is used to start build
.IF "$(CPUNAME)" == "X86_64"
ARCH_FLAGS*=
.ENDIF

# Compiler flags for enabling optimizations
.IF "$(PRODUCT)"!=""
CFLAGSOPT=-O2 -fno-strict-aliasing		# optimizing for products
.ELSE 	# "$(PRODUCT)"!=""
CFLAGSOPT=   					# no optimizing for non products
.ENDIF	# "$(PRODUCT)"!=""

BUILD64=1
