# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,rust_uno-example))

$(eval $(call gb_Library_add_exception_objects,rust_uno-example, \
    rust_uno/example/example \
))

$(eval $(call gb_Library_set_componentfile,rust_uno-example,rust_uno/example/example,rust_uno-example))

$(eval $(call gb_Library_set_external_code,rust_uno-example))

$(eval $(call gb_Library_use_externals,rust_uno-example, \
    boost_headers \
))

$(eval $(call gb_Library_use_libraries,rust_uno-example, \
    cppu \
    cppuhelper \
    sal \
))

$(eval $(call gb_Library_use_sdk_api,rust_uno-example))

$(call gb_Library_get_target,rust_uno-example): $(call gb_CustomTarget_get_target,rust_uno/cargo)

ifneq ($(filter WNT,$(OS_FOR_BUILD)),)
    # Windows-specific flags
    $(eval $(call gb_Library_add_libs,rust_uno-example,\
        $(SRCDIR)/rust_uno/target/release/rust_uno.lib \
    ))
else
    # Unix/Linux with RPATH
    $(eval $(call gb_Library_add_ldflags,rust_uno-example,\
        -Wl$(COMMA)-rpath$(COMMA)\$$$$ORIGIN \
    ))
    $(eval $(call gb_Library_add_libs,rust_uno-example,\
        -L$(SRCDIR)/rust_uno/target/release -lrust_uno \
    ))
endif

# vim: set noet sw=4 ts=4:
