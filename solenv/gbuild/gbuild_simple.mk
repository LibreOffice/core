# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2011 Oracle and/or its affiliates.
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

# this is a subset of gbuild functionality, for use in custom makefiles

include $(GBUILDDIR)/Output.mk

# BuildDirs uses the Output functions already
include $(GBUILDDIR)/BuildDirs.mk

# Presumably the common parts in gbuild.mk and gbuild_simple.mk should
# be factored out into one file instead of duplicating, but... perhaps
# wait until this stabilizes a bit and we know with more certainty
# what is needed in both.

# Or alternatively: Just mark these variables for export in gbuild.mk?
# I think any use of gbuild_simple.mk is in a sub-make under one that
# uses gbuild.mk anyway.

# gb_PRODUCT is used by windows.mk to decide which C/C++ runtime to
# link with.
ifneq ($(strip $(PRODUCT)$(product)),)
gb_PRODUCT := $(true)
else
ifneq ($(strip $(product)),)
gb_PRODUCT := $(true)
else
gb_PRODUCT := $(false)
endif
endif

# These are useful, too, for stuff built in "custom" Makefiles
ifneq ($(strip $(ENABLE_SYMBOLS)$(enable_symbols)),)
gb_SYMBOL := $(true)
else
gb_SYMBOL := $(false)
endif

gb_DEBUGLEVEL := 0
ifneq ($(strip $(DEBUG)$(debug)),)
gb_DEBUGLEVEL := 1
endif

ifneq ($(strip $(DBGLEVEL)$(dbglevel)),)
ifneq ($(strip $(dbglevel)),)
gb_DEBUGLEVEL := $(strip $(dbglevel))
else
gb_DEBUGLEVEL := $(strip $(DBGLEVEL))
endif
endif

ifneq ($(gb_DEBUGLEVEL),0)
gb_SYMBOL := $(true)
endif

include $(GBUILDDIR)/Helper.mk

ifeq ($(OS),LINUX)
include $(GBUILDDIR)/platform/linux.mk
else ifeq ($(OS),WNT)
ifeq ($(COM),GCC)
include $(GBUILDDIR)/platform/winmingw.mk
else
include $(GBUILDDIR)/platform/windows.mk
endif
else ifeq ($(OS),SOLARIS)
include $(GBUILDDIR)/platform/solaris.mk
else ifeq ($(OS),MACOSX)
include $(GBUILDDIR)/platform/macosx.mk
else ifeq ($(OS),OPENBSD)
include $(GBUILDDIR)/platform/openbsd.mk
else ifeq ($(OS),FREEBSD)
include $(GBUILDDIR)/platform/freebsd.mk
else ifeq ($(OS),NETBSD)
include $(GBUILDDIR)/platform/netbsd.mk
else ifeq ($(OS),DRAGONFLY)
include $(GBUILDDIR)/platform/dragonfly.mk
else ifeq ($(OS),IOS)
include $(GBUILDDIR)/platform/ios.mk
else ifeq ($(OS),ANDROID)
include $(GBUILDDIR)/platform/android.mk
else
$(eval $(call gb_Output_error,Unsupported OS: $(OS)))
endif

# vim: set noet sw=4 ts=4:
