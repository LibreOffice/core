# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,spsupp_x86))

$(eval $(call gb_Library_set_x86,spsupp_x86,YES))

# when building with link-time optimisation on, we need to turn it off for the helper
ifeq ($(ENABLE_LTO),TRUE)
$(eval $(call gb_Library_add_cxxflags,spsupp_x86,\
	-GL- \
))
$(eval $(call gb_Library_add_ldflags,spsupp_x86,\
	-LTCG:OFF \
))
endif

$(eval $(call gb_Library_use_custom_headers,spsupp_x86,\
	shell/source/win32/spsupp/idl \
))

$(eval $(call gb_Library_set_include,spsupp_x86,\
	-I$(SRCDIR)/shell/inc/spsupp \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_use_system_win32_libs,spsupp_x86,\
	advapi32 \
	kernel32 \
	ole32 \
	oleaut32 \
	user32 \
))

$(eval $(call gb_Library_add_nativeres,spsupp_x86,spsupp))

$(eval $(call gb_Library_add_ldflags,spsupp_x86,\
	/DEF:$(SRCDIR)/shell/source/win32/spsupp/spsupp.def \
))

$(eval $(call gb_Library_add_exception_objects,spsupp_x86,\
    shell/source/win32/spsupp/COMOpenDocuments \
    shell/source/win32/spsupp/registrar \
    shell/source/win32/spsupp/spsuppClassFactory \
    shell/source/win32/spsupp/spsuppServ \
))

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
