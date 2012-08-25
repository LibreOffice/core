# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Initial Developer of the Original Code is
# 	Peter Foley <pefoley2@verizon.net>
# Portions created by the Initial Developer are Copyright (C) 2011 the
# Initial Developer. All Rights Reserved.
#
# Major Contributor(s):
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.
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

$(eval $(call gb_Library_add_nativeres,so_activex_x64,activex_res))

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

ifneq ($(USE_DEBUG_RUNTIME),)
$(eval $(call gb_Library_add_libs,so_activex_x64,\
	$(ATL_LIB)/amd64/atlsd.lib \
))
else
$(eval $(call gb_Library_add_libs,so_activex_x64,\
	$(ATL_LIB)/amd64/atls.lib \
))
endif

# vim:set shiftwidth=4 softtabstop=4 expandtab:
