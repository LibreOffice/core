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

# extend for JDK include (seems only needed in setsolar env?)
SOLARINC += $(JDKINCS)

OUTDIR := $(SOLARVERSION)/$(INPATH)
WORKDIR := $(SOLARVERSION)/$(INPATH)/workdir

# Override for SetupLocal
ifneq ($(and $(gb_LOCALBUILDDIR),$(wildcard $(gb_LOCALBUILDDIR)/SetupLocal.mk)),)
include $(gb_LOCALBUILDDIR)/SetupLocal.mk
endif

ifeq ($(strip $(gb_REPOS)),)
gb_REPOS := $(SOLARSRC)
endif

# HACK
# unixify windoze paths
ifeq ($(OS),WNT)
override WORKDIR := $(shell cygpath -u $(WORKDIR))
override OUTDIR := $(shell cygpath -u $(OUTDIR))
override gb_REPOS := $(shell cygpath -u $(gb_REPOS))
endif

REPODIR := $(patsubst %/,%,$(dir $(firstword $(gb_REPOS))))

ifeq ($(filter setuplocal removelocal,$(MAKECMDGOALS)),)
ifneq ($(filter-out $(foreach repo,$(gb_REPOS),$(realpath $(repo))/%),$(realpath $(firstword $(MAKEFILE_LIST)))),)
$(eval $(call gb_Output_error,The initial makefile $(realpath $(firstword $(MAKEFILE_LIST))) is not in the repositories $(foreach repo,$(gb_REPOS),$(realpath $(repo))).,ALL))
endif
endif

# vim: set noet sw=4 ts=4:
