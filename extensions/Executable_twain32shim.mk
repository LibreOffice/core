# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,twain32shim))

$(eval $(call gb_Executable_set_targettype_gui,twain32shim,YES))

$(eval $(call gb_Executable_set_x86,twain32shim,YES))

# when building with link-time optimisation on, we need to turn it off for the helper
ifeq ($(ENABLE_LTO),TRUE)
$(eval $(call gb_Executable_add_cxxflags,twain32shim,\
	-GL- \
))
$(eval $(call gb_Executable_add_ldflags,twain32shim,\
	-LTCG:OFF \
))
endif

$(eval $(call gb_Executable_use_externals,twain32shim,\
    sane_headers \
))

$(eval $(call gb_Executable_set_include,twain32shim,\
    -I$(SRCDIR)/extensions/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_Executable_add_exception_objects,twain32shim,\
    extensions/source/scanner/twain32shim \
))

$(eval $(call gb_Executable_use_system_win32_libs,twain32shim,\
    Ole32 \
    shell32 \
))

$(eval $(call gb_Executable_add_default_nativeres,twain32shim))

# vim:set noet sw=4 ts=4:
