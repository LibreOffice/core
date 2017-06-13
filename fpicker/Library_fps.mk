# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,fps))

$(eval $(call gb_Library_use_custom_headers,fps,\
	officecfg/registry \
))

$(eval $(call gb_Library_add_nativeres,fps,fps/Fps))

$(eval $(call gb_Library_set_componentfile,fps,fpicker/source/win32/fps))

$(eval $(call gb_Library_set_include,fps,\
    $$(INCLUDE) \
    -I$(SRCDIR)/fpicker/inc \
))

$(eval $(call gb_Library_use_external,fps,boost_headers))

$(eval $(call gb_Library_use_sdk_api,fps))

$(eval $(call gb_Library_add_defs,fps,\
	-D_UNICODE \
	-DUNICODE \
))

$(eval $(call gb_Library_use_libraries,fps,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	i18nlangtag \
	tl \
	utl \
	vcl \
))

$(eval $(call gb_Library_use_system_win32_libs,fps,\
	advapi32 \
	comdlg32 \
	gdi32 \
	kernel32 \
	ole32 \
	oleaut32 \
	shell32 \
	uuid \
))

ifeq ($(COM),MSC)
$(eval $(call gb_Library_add_libs,fps,\
	Delayimp.lib /DELAYLOAD:shell32.dll \
))
endif

$(eval $(call gb_Library_add_exception_objects,fps,\
	fpicker/source/win32/filepicker/asyncrequests \
	fpicker/source/win32/filepicker/FilterContainer \
	fpicker/source/win32/filepicker/FPentry \
	fpicker/source/win32/filepicker/VistaFilePicker \
	fpicker/source/win32/filepicker/VistaFilePickerEventHandler \
	fpicker/source/win32/filepicker/VistaFilePickerImpl \
	fpicker/source/win32/folderpicker/FolderPicker \
	fpicker/source/win32/folderpicker/MtaFop \
	fpicker/source/win32/folderpicker/WinFOPImpl \
	fpicker/source/win32/misc/resourceprovider \
	fpicker/source/win32/misc/WinImplHelper \
))

# vim: set noet sw=4 ts=4:
