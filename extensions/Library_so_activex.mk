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

$(eval $(call gb_Library_Library,so_activex))

$(eval $(call gb_Library_use_custom_headers,so_activex,extensions/source/activex/idl))

$(eval $(call gb_Library_set_include,so_activex,\
	$$(INCLUDE) \
	$(foreach i,$(ATL_INCLUDE), -I$(i)) \
))

$(eval $(call gb_Library_use_sdk_api,so_activex))

$(eval $(call gb_Library_add_nativeres,so_activex,activex_res))

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

ifneq ($(USE_DEBUG_RUNTIME),)
$(eval $(call gb_Library_add_libs,so_activex,\
	$(ATL_LIB)/atlsd.lib \
))
else
$(eval $(call gb_Library_add_libs,so_activex,\
	$(ATL_LIB)/atls.lib \
))
endif

# vim:set shiftwidth=4 softtabstop=4 expandtab:
