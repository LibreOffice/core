# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,spsupp_helper))

$(eval $(call gb_Executable_set_targettype_gui,spsupp_helper,YES))

$(eval $(call gb_Executable_set_include,spsupp_helper,\
    -I$(SRCDIR)/shell/inc/spsupp \
    $$(INCLUDE) \
))

$(eval $(call gb_Executable_use_sdk_api,spsupp_helper))

$(eval $(call gb_Executable_use_libraries,spsupp_helper,\
	i18nlangtag \
	sal \
	utl \
))

$(eval $(call gb_Executable_add_exception_objects,spsupp_helper,\
    shell/source/win32/spsupp/spsuppHelper \
))

$(eval $(call gb_Executable_add_nativeres,spsupp_helper,spsupp_dlg))

$(eval $(call gb_Executable_use_system_win32_libs,spsupp_helper,\
    shell32 \
))

# vim:set noet sw=4 ts=4:
