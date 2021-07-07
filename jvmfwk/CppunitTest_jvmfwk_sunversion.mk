# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,jvmfwk_sunversion))

$(eval $(call gb_CppunitTest_add_exception_objects,jvmfwk_sunversion, \
    jvmfwk/qa/unit/sunversion \
))

$(eval $(call gb_CppunitTest_set_include,jvmfwk_sunversion, \
    $$(INCLUDE) \
    -I$(SRCDIR) \
))

$(eval $(call gb_CppunitTest_use_externals,jvmfwk_sunversion, \
    libxml2 \
))

$(eval $(call gb_CppunitTest_use_libraries,jvmfwk_sunversion, \
    sal \
    salhelper \
))

$(eval $(call gb_CppunitTest_use_library_objects,jvmfwk_sunversion, \
    jvmfwk \
))

ifeq ($(OS),MACOSX)
$(eval $(call gb_CppunitTest_use_system_darwin_frameworks,jvmfwk_sunversion, \
    Foundation \
))
endif

# vim: set noet sw=4 ts=4:
