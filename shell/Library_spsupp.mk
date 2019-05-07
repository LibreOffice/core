# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,spsupp))

$(eval $(call gb_Library_use_custom_headers,spsupp,\
	shell/source/win32/spsupp/idl \
))

$(eval $(call gb_Library_set_include,spsupp,\
	-I$(SRCDIR)/shell/inc/spsupp \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_use_sdk_api,spsupp))

$(eval $(call gb_Library_use_libraries,spsupp,\
	i18nlangtag \
	sal \
	utl \
))

$(eval $(call gb_Library_use_system_win32_libs,spsupp,\
	advapi32 \
	kernel32 \
	ole32 \
	oleaut32 \
	user32 \
))

$(eval $(call gb_Library_add_nativeres,spsupp,spsupp))

$(eval $(call gb_Library_add_ldflags,spsupp,\
	/DEF:$(SRCDIR)/shell/source/win32/spsupp/spsupp.def \
))

$(eval $(call gb_Library_add_exception_objects,spsupp,\
    shell/source/win32/spsupp/COMOpenDocuments \
    shell/source/win32/spsupp/registrar \
    shell/source/win32/spsupp/spsuppClassFactory \
    shell/source/win32/spsupp/spsuppEditOrRODlg \
    shell/source/win32/spsupp/spsuppServ \
))

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
