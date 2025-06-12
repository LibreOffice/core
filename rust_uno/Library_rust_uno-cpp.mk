# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,rust_uno-cpp))

$(eval $(call gb_Library_use_libraries,rust_uno-cpp, \
    cppu \
    cppuhelper \
    sal \
))

$(eval $(call gb_Library_use_sdk_api,rust_uno-cpp))

# Add the uno_bootstrap.cxx file for bootstrap functionality
$(eval $(call gb_Library_add_exception_objects,rust_uno-cpp, \
    rust_uno/uno_bootstrap \
))

# Combined generated files instead of thousands of individual files
rust_uno_generated_cxx = \
    rust_uno_bindings

define rust_uno_add_generated_cxx
$(gb_CustomTarget_workdir)/rust_uno/rustmaker/cpp/$(1).cxx: \
        $(call gb_CustomTarget_get_target,rust_uno/rustmaker)

$(eval $(call gb_Library_add_generated_exception_objects,rust_uno-cpp, \
    CustomTarget/rust_uno/rustmaker/cpp/$(1) \
))

endef

$(foreach gencxx,$(rust_uno_generated_cxx),$(eval $(call rust_uno_add_generated_cxx,$(gencxx))))

# vim: set noet sw=4 ts=4:
