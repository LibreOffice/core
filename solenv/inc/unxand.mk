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

# mk file for Android Linux ARM using GCC, please make generic modifications to unxgcc.mk

CDEFAULTOPT=-Os
.INCLUDE : unxgcc.mk
.IF "$(CPU)" == "I"
CDEFS+=-DX86
.ELIF "$(CPU)" == "M"
CDEFS+=-DMIPS
.ELSE
CDEFS+=-DARM32
.ENDIF
CFLAGS+=-fno-omit-frame-pointer

# Override some macros set by unxgcc.mk

# _PTHREADS and _REENTRANT are meaningful in glibc headers only, and
# in the NDK in stlport, which we don't use.
CDEFS !:= $(subst,-D_PTHREADS, $(CDEFS))
CDEFS !:= $(subst,-D_REENTRANT, $(CDEFS))

# We don't build any "tool" style programs for non-desktop OSes like
# Android. Just unit tests and GUI programs. (Well, that is in
# theory. In reality any actual "app" with a GUI for Android would be
# written and built in a totally Android-specific way, of course.)

# Thus for Android we never build executable programs, just shared
# libraries that the NativeActivity Java code will load.

LINKFLAGSAPPGUI=-shared -Wl,--as-needed -Wl,--no-add-needed
LINKFLAGSAPPCUI=-shared -Wl,--as-needed -Wl,--no-add-needed

LINKFLAGSSHLGUI+= -Wl,--as-needed -Wl,--no-add-needed
LINKFLAGSSHLCUI+= -Wl,--as-needed -Wl,--no-add-needed

STDLIBGUIMT+=-llog -landroid -lgnustl_shared
STDLIBCUIMT+=-llog -landroid -lgnustl_shared
STDSHLGUIMT+=-llog -landroid -lgnustl_shared
STDSHLCUIMT+=-llog -landroid -lgnustl_shared

# All shared libraries we build must start with "lib" because
# otherwise the Android package installer will not unpack them from
# the .apk into the app's lib directory.
ENFORCEDSHLPREFIX=lib

# No DT_RPATH or DT_RUNPATH support in the Bionic dynamic linker so
# don't bother generating such.

LINKFLAGSRUNPATH_URELIB=
LINKFLAGSRUNPATH_UREBIN=
LINKFLAGSRUNPATH_OOO=
LINKFLAGSRUNPATH_SDKBIN=

DLLPOST=.a
