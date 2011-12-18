# -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
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

$(SRCDIR)/extensions/source/activex/so_activex.cxx: $(WORKDIR)/CustomTarget/so_activex/so_activex.tlb

$(WORKDIR)/CustomTarget/so_activex/so_activex.tlb: $(SRCDIR)/extensions/source/activex/so_activex.idl
	mkdir -p $(WORKDIR)/CustomTarget/so_activex
	midl.exe -out $(call gb_Helper_convert_native,$(WORKDIR)/CustomTarget/so_activex) -Oicf $(call gb_Helper_convert_native,$<) \
	$(foreach i,$(SOLARINC_FOR_BUILD), $(patsubst -I,/I,$(i)))

$(eval $(call gb_Library_set_include,so_activex,\
	$$(INCLUDE) \
	-I$(WORKDIR)/CustomTarget/so_activex \
	$(foreach i,$(ATL_INCLUDE), -I$(i)) \
))

$(eval $(call gb_Library_add_api,so_activex,\
	offapi \
	udkapi \
))

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
	/DEF:$(call gb_Helper_convert_native,$(SRCDIR)/extensions/source/activex/so_activex.def) \
))

$(eval $(call gb_Library_add_linked_libs,so_activex,\
	uuid \
	advapi32 \
	ole32 \
	oleaut32 \
	gdi32 \
	urlmon \
	shlwapi \
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

$(call gb_Library_get_clean_target,so_activex): idlclean

idlclean:
	rm -rf $(WORKDIR)/CustomTarget/so_activex

# vim:set shiftwidth=4 softtabstop=4 expandtab:
