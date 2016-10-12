# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

module_directory:=$(dir $(realpath $(firstword $(MAKEFILE_LIST))))

ifeq (,$(BUILDDIR))
gb_partial_build__makefile_dir=$(dir $(abspath $(firstword $(MAKEFILE_LIST))))
BUILDDIR := $(gb_partial_build__makefile_dir)../../../..
endif

gb_GBUILDSELFTEST=t

include $(module_directory)/../../../gbuild/partial_build.mk

# vim: set noet sw=4 ts=4:
