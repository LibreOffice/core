#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: unxfbsdx.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: kz $ $Date: 2006-07-19 09:27:16 $
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

# platform specific identifier for shared libs
DLLPOSTFIX=fx
