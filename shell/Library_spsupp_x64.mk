# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,spsupp_x64))

$(eval $(call gb_Library_set_x64,spsupp_x64,YES))

$(eval $(call gb_Library_use_custom_headers,spsupp_x64,\
	shell/source \
	shell/source/win32/spsupp/idl \
))

$(eval $(call gb_Library_set_include,spsupp_x64,\
	-I$(SRCDIR)/shell/inc/spsupp \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_add_defs,spsupp_x64,\
	-DUNICODE \
	-D_UNICODE \
))

$(eval $(call gb_Library_use_system_win32_libs,spsupp_x64,\
	advapi32 \
	kernel32 \
	ole32 \
	oleaut32 \
	user32 \
))

$(eval $(call gb_Library_add_nativeres,spsupp_x64,spsupp))

$(eval $(call gb_Library_add_ldflags,spsupp_x64,\
	/DEF:$(SRCDIR)/shell/source/win32/spsupp/spsupp.def \
))

$(eval $(call gb_Library_add_x64_generated_exception_objects,spsupp_x64,\
    CustomTarget/shell/source/win32/spsupp/COMOpenDocuments \
    CustomTarget/shell/source/win32/spsupp/registrar \
    CustomTarget/shell/source/win32/spsupp/spsuppClassFactory \
    CustomTarget/shell/source/win32/spsupp/spsuppServ \
))

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
