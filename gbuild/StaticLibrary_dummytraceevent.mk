# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# Copyright the Collabora Online contributors.
#
# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,dummytraceevent))

$(eval $(call gb_StaticLibrary_set_generated_cxx_suffix,dummytraceevent,cpp))

$(eval $(call gb_StaticLibrary_set_generated_cxx_base,dummytraceevent,$(SRCDIR)/..))

$(eval $(call gb_StaticLibrary_set_generated_warnings_as_errors,dummytraceevent))

$(eval $(call gb_StaticLibrary_set_include,dummytraceevent, \
    -I$(or $(ONLINE.BUILDDIR),$(realpath $(BUILDDIR)/..)) \
    -I$(SRCDIR)/.. \
    $$(INCLUDE) \
    -I$(gb_UnpackedTarball_workdir)/poco/include \
))

$(eval $(call gb_StaticLibrary_add_generated_exception_objects,dummytraceevent, \
    common/DummyTraceEventEmitter \
))

#TODO: For now, engine builds Poco without the --enable-dbgutil debug libstdc++:
$(eval $(call gb_StaticLibrary_add_defs,dummytraceevent,-U_GLIBCXX_DEBUG))

# vim: set noet sw=4 ts=4:
