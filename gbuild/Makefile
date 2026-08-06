# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# Copyright the Collabora Online contributors.
#
# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

module_directory:=$(dir $(realpath $(firstword $(MAKEFILE_LIST))))

# In the monorepo the engine sources sit next to this directory.  A build from
# the distribution tarball ships no engine, so take the engine source directory
# from the configuration of the engine build tree we were pointed at.
engine_srcdir:=$(module_directory)../engine
ifeq (,$(wildcard $(engine_srcdir)/solenv/gbuild/partial_build.mk))
engine_srcdir:=$(shell sed -n 's/^export SRCDIR=//p' $(BUILDDIR)/config_host.mk)
endif

include $(engine_srcdir)/solenv/gbuild/partial_build.mk

# vim: set noet sw=4 ts=4:
