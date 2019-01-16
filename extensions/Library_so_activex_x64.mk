# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#

$(eval $(call gb_Library_Library,so_activex_x64))

$(eval $(call gb_Library_set_x64,so_activex_x64,YES))

$(eval $(call gb_Library_use_custom_headers,so_activex_x64,extensions/source/activex/idl))

$(eval $(call gb_Library_set_include,so_activex_x64,\
	$$(INCLUDE) \
	-I$(SRCDIR)/extensions/source/activex \
	$(foreach i,$(ATL_INCLUDE), -I$(i)) \
))

$(eval $(call gb_Library_use_sdk_api,so_activex_x64))

$(eval $(call gb_Library_add_nativeres,so_activex_x64,so_activex))

$(eval $(call gb_Library_add_x64_generated_exception_objects,so_activex_x64,\
	CustomTarget/extensions/source/activex/so_activex \
	CustomTarget/extensions/source/activex/SOActiveX \
	CustomTarget/extensions/source/activex/SOComWindowPeer \
	CustomTarget/extensions/source/activex/SODispatchInterceptor \
	CustomTarget/extensions/source/activex/SOActionsApproval \
	CustomTarget/extensions/source/activex/StdAfx2 \
))

$(eval $(call gb_Library_add_ldflags,so_activex_x64,\
	/DEF:$(SRCDIR)/extensions/source/activex/so_activex.def \
	-LIBPATH:$(ATL_LIB)/amd64 \
))

$(eval $(call gb_Library_use_system_win32_libs,so_activex_x64,\
	advapi32 \
	gdi32 \
	ole32 \
	oleaut32 \
	shlwapi \
	urlmon \
	uuid \
))

$(eval $(call gb_Library_add_libs,so_activex_x64,\
	$(subst /x86,/x64,$(ATL_LIB)/atls.lib)) \
)

# vim:set noet sw=4 ts=4:
