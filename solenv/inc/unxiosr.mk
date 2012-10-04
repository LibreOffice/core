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

##########################################################################
# Platform MAKEFILE for iOS, both devices and the simulator
##########################################################################

PROCESSOR_DEFINES=-DARM32

# flags to enable build with symbols
CFLAGSENABLESYMBOLS=-g

ASM=
AFLAGS=
LINKOUTPUT_FILTER=

CFLAGS=-fmessage-length=0 -c $(EXTRA_CFLAGS)

# ---------------------------------
#  Compilation flags
# ---------------------------------
# Normal C compilation flags
CFLAGSCC=-pipe -fsigned-char $(ARCH_FLAGS)

# Normal Objective C compilation flags
OBJCFLAGS=-fexceptions -fobjc-abi-version=2 -fobjc-legacy-dispatch -D__IPHONE_OS_VERSION_MIN_REQUIRED=40300

OBJCXXFLAGS:=-x objective-c++ $(OBJCFLAGS)

# Comp Flags for files that need exceptions enabled (C and C++)
CFLAGSEXCEPTIONS=-fexceptions
.IF "$(dbgutil)"==""
.IF "$(COM_GCC_IS_CLANG)" != "TRUE"
CFLAGSEXCEPTIONS+=-fno-enforce-eh-specs
.ENDIF
.ENDIF

# Comp Flags for files that do not need exceptions enabled (C and C++)
CFLAGS_NO_EXCEPTIONS=-fno-exceptions

# Normal C++ compilation flags
CFLAGSCXX=-pipe -fsigned-char $(ARCH_FLAGS) -Wno-ctor-dtor-privacy
.IF "$(HAVE_THREADSAFE_STATICS)" != "TRUE"
CFLAGSCXX += -fno-threadsafe-statics
.END

# No PIC needed as we don't build dynamic objects
PICSWITCH:=
# Other flags
CFLAGSOBJGUIMT=$(PICSWITCH) -fno-common
CFLAGSOBJCUIMT=$(PICSWITCH) -fno-common
CFLAGSSLOGUIMT=$(PICSWITCH) -fno-common
CFLAGSSLOCUIMT=$(PICSWITCH) -fno-common

# Flag for including debugging information in object files
CFLAGSDEBUG=-g
CFLAGSDBGUTIL=

# Flag to specify output file to compiler/linker
CFLAGSOUTOBJ=-o

# ---------------------------------
#  Optimization flags
# ---------------------------------
CFLAGSOPT=-O2 -fno-strict-aliasing
CFLAGSNOOPT=-O0

# -Wshadow does not work for C with nested uses of pthread_cleanup_push:
# -Wshadow does not work for C++ as /usr/include/c++/4.0.0/ext/hashtable.h
# l. 717 contains a declaration of __cur2 shadowing the declaration at l. 705,
# in template code for which a #pragma gcc system_header would not work:
# -Wextra doesn not work for gcc-3.3
CFLAGSWARNCC=-Wall -Wendif-labels
CFLAGSWARNCXX=$(CFLAGSWARNCC) -Wno-ctor-dtor-privacy -Wno-non-virtual-dtor
CFLAGSWALLCC=$(CFLAGSWARNCC)
CFLAGSWALLCXX=$(CFLAGSWARNCXX)
CFLAGSWERRCC=-Werror -DLIBO_WERROR

LINK*=$(CXX)
LINKC*=$(CC)

LINKFLAGSDEFS*=-Wl,-multiply_defined,suppress

# Tag to identify an output file as a library
DLLPRE=lib
# We don't use dynamic shared libraries on iOS
DLLPOST=.a

STDLIBCUIMT=-framework UIKit -framework Foundation -framework CoreText -framework CoreGraphics -framework CoreFoundation -Xlinker -objc_abi_version -Xlinker 2
STDLIBGUIMT=$(STDLIBCUIMT)
STDSHLCUIMT=
STDSHLGUIMT=

LIBMGR=ar
LIBFLAGS=-r

IMPLIB=:
IMPLIBFLAGS=

MAPSYM=:
MAPSYMFLAGS=

RC=:
RCFLAGS=
RCLINK=
RCLINKFLAGS=
RCSETVERSION=

OOO_LIBRARY_PATH_VAR = DYLD_LIBRARY_PATH
