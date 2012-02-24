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

GBUILDDIR:=$(SRC_ROOT)/solenv/gbuild

# this is a subset of gbuild functionality, for use in custom makefiles

COMMA :=,

include $(GBUILDDIR)/Output.mk

# BuildDirs overrides *DIR variables for Windows
include $(GBUILDDIR)/BuildDirs.mk

# Presumably the common parts in gbuild.mk and gbuild_simple.mk should
# be factored out into one file instead of duplicating, but... perhaps
# wait until this stabilizes a bit and we know with more certainty
# what is needed in both.

# Or alternatively: Just mark these variables for export in gbuild.mk?
# I think any use of gbuild_simple.mk is in a sub-make under one that
# uses gbuild.mk anyway.

ifneq ($(strip $(PRODUCT)$(product)),)
gb_PRODUCT := $(true)
else
gb_PRODUCT := $(false)
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
include $(GBUILDDIR)/Tempfile.mk

# Include platform/cpu/compiler specific config/definitions
include $(GBUILDDIR)/platform/$(OS)_$(CPUNAME)_$(COM).mk

ifeq ($(SYSTEM_PYTHON),YES)
gb_PYTHONTARGET :=
gb_PYTHON := $(PYTHON)
else ifeq ($(OS),MACOSX)
#fixme: remove this MACOSX ifeq branch by filling in gb_PYTHON_PRECOMMAND in
#gbuild/platform/macosx.mk correctly for mac, e.g. PYTHONPATH and PYTHONHOME
#dirs for in-tree internal python
gb_PYTHONTARGET :=
gb_PYTHON := $(PYTHON)
else ifeq ($(DISABLE_PYTHON),TRUE)
# Build-time python
gb_PYTHON := python
else
gb_PYTHONTARGET := $(OUTDIR)/bin/python
gb_PYTHON := $(gb_PYTHON_PRECOMMAND) $(gb_PYTHONTARGET)
endif

# vim: set noet sw=4 ts=4:
