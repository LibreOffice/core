# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2009 by Sun Microsystems, Inc.
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

# The real targets are in solenv/gbuild/Module.mk, resp.
# solenv/gbuild/extensions/post_BuildplTargets.mk, this file only forwards
# the calls - please look there if you want to edit what this toplevel
# Makefile does.

SRCDIR:=$(patsubst %/,%,$(dir $(realpath $(firstword $(MAKEFILE_LIST)))))

ifeq ($(strip $(gb_SourceEnvAndRecurse_STAGE)),)
include $(SRCDIR)/solenv/gbuild/source_and_rerun.mk
endif

ifeq ($(gb_SourceEnvAndRecurse_STAGE),reconfigure)
SOLARENV:=$(SRCDIR)/solenv
endif

ifneq ($(filter reconfigure gbuild buildpl,$(gb_SourceEnvAndRecurse_STAGE)),)
include $(SOLARENV)/gbuild/gbuild.mk
$(eval $(call gb_Module_make_global_targets,$(wildcard $(SRCDIR)/RepositoryModule_*.mk)))
endif
# vim: set noet sw=4 ts=4:
