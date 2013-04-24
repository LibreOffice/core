# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#

$(eval $(call gb_Library_Library,so_activex))

$(eval $(call gb_Library_use_custom_headers,so_activex,extensions/source/activex/idl))

$(eval $(call gb_Library_set_include,so_activex,\
	$$(INCLUDE) \
	$(foreach i,$(ATL_INCLUDE), -I$(i)) \
))

$(eval $(call gb_Library_use_sdk_api,so_activex))

$(eval $(call gb_Library_add_nativeres,so_activex,so_activex))

$(eval $(call gb_Library_add_exception_objects,so_activex,\
	extensions/source/activex/so_activex \
	extensions/source/activex/SOActiveX \
	extensions/source/activex/SOComWindowPeer \
	extensions/source/activex/SODispatchInterceptor \
	extensions/source/activex/SOActionsApproval \
	extensions/source/activex/StdAfx2 \
))

$(eval $(call gb_Library_add_ldflags,so_activex,\
	/DEF:$(SRCDIR)/extensions/source/activex/so_activex.def \
	-LIBPATH:$(ATL_LIB) \
))

$(eval $(call gb_Library_use_system_win32_libs,so_activex,\
	advapi32 \
	gdi32 \
	ole32 \
	oleaut32 \
	shlwapi \
	urlmon \
	uuid \
))

$(eval $(call gb_Library_add_libs,so_activex,\
	$(ATL_LIB)/$(if $(MSVC_USE_DEBUG_RUNTIME),atlsd.lib,atls.lib) \
))

# vim:set noet sw=4 ts=4:
