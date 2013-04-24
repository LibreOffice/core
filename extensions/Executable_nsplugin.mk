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

$(eval $(call gb_Executable_Executable,nsplugin))

$(eval $(call gb_Executable_use_external,nsplugin,boost_headers))

$(eval $(call gb_Executable_use_sdk_api,nsplugin))

$(eval $(call gb_Executable_add_exception_objects,nsplugin,\
	extensions/source/nsplugin/source/so_closelistener \
	extensions/source/nsplugin/source/so_instance \
	extensions/source/nsplugin/source/so_main \
))

$(eval $(call gb_Executable_use_static_libraries,nsplugin,\
	npsoenv \
))

$(eval $(call gb_Executable_add_libs,nsplugin,\
	$(if $(filter $(OS),LINUX), \
		-ldl \
	) \
	$(if $(filter $(OS),SOLARIS), \
		-lnsl \
		-lsocket \
	) \
))

$(eval $(call gb_Executable_use_libraries,nsplugin,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
))

ifeq ($(OS),WNT)
$(eval $(call gb_Executable_add_exception_objects,nsplugin,\
	extensions/source/nsplugin/source/nsp_windows \
))

$(eval $(call gb_Executable_use_static_libraries,nsplugin,\
	ooopathutils \
))

$(eval $(call gb_Executable_use_system_win32_libs,nsplugin,\
	advapi32 \
	comdlg32 \
	gdi32 \
	kernel32 \
	ole32 \
	oleaut32 \
	shell32 \
	user32 \
	uuid \
	winspool \
	ws2_32 \
))
endif # WNT

# vim:set noet sw=4 ts=4:
