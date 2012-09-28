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
# Major Contributor(s):
# Copyright (C) 2011 Matúš Kukan <matus.kukan@gmail.com> (initial developer)
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_Library_Library,fps))

$(eval $(call gb_Library_add_nativeres,fps,fps/src))

$(eval $(call gb_Library_set_componentfile,fps,fpicker/source/win32/fps))

$(eval $(call gb_Library_use_packages,fps,\
	vcl_inc \
))

$(eval $(call gb_Library_use_sdk_api,fps))

$(eval $(call gb_Library_add_defs,fps,\
	-D_UNICODE \
	-DUNICODE \
	-U_WIN32_WINNT \
	-D_WIN32_WINNT=0x0600 \
))

$(eval $(call gb_Library_use_libraries,fps,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	tl \
	utl \
	vcl \
	$(gb_UWINAPI) \
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
	fpicker/source/win32/filepicker/asynceventnotifier \
	fpicker/source/win32/filepicker/asyncrequests \
	fpicker/source/win32/filepicker/controlaccess \
	fpicker/source/win32/filepicker/controlcommand \
	fpicker/source/win32/filepicker/customcontrolcontainer \
	fpicker/source/win32/filepicker/customcontrol \
	fpicker/source/win32/filepicker/customcontrolfactory \
	fpicker/source/win32/filepicker/dialogcustomcontrols \
	fpicker/source/win32/filepicker/dibpreview \
	fpicker/source/win32/filepicker/FileOpenDlg \
	fpicker/source/win32/filepicker/FilePicker \
	fpicker/source/win32/filepicker/filepickereventnotification \
	fpicker/source/win32/filepicker/filepickerstate \
	fpicker/source/win32/filepicker/FilterContainer \
	fpicker/source/win32/filepicker/FPentry \
	fpicker/source/win32/filepicker/getfilenamewrapper \
	fpicker/source/win32/filepicker/helppopupwindow \
	fpicker/source/win32/filepicker/previewadapter \
	fpicker/source/win32/filepicker/previewbase \
	fpicker/source/win32/filepicker/SolarMutex \
	fpicker/source/win32/filepicker/VistaFilePicker \
	fpicker/source/win32/filepicker/VistaFilePickerEventHandler \
	fpicker/source/win32/filepicker/VistaFilePickerImpl \
	fpicker/source/win32/filepicker/WinFileOpenImpl \
	fpicker/source/win32/folderpicker/FolderPicker \
	fpicker/source/win32/folderpicker/MtaFop \
	fpicker/source/win32/folderpicker/WinFOPImpl \
	fpicker/source/win32/misc/AutoBuffer \
	fpicker/source/win32/misc/resourceprovider \
	fpicker/source/win32/misc/WinImplHelper \
))
#	fpicker/source/win32/filepicker/PreviewCtrl \

# vim: set noet sw=4 ts=4:
