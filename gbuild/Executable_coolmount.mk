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

$(eval $(call gb_Executable_Executable,coolmount))

$(eval $(call gb_Executable_set_generated_cxx_suffix,coolmount,cpp))

$(eval $(call gb_Executable_set_generated_cxx_base,coolmount,$(SRCDIR)/..))

$(eval $(call gb_Executable_set_generated_warnings_as_errors,coolmount))

$(eval $(call gb_Executable_set_include,coolmount, \
    -I$(or $(ONLINE.BUILDDIR),$(realpath $(BUILDDIR)/..)) \
    -I$(SRCDIR)/.. \
    $$(INCLUDE) \
))

$(eval $(call gb_Executable_add_generated_exception_objects,coolmount, \
    tools/mount \
    common/CoolMount \
))

# vim: set noet sw=4 ts=4:
