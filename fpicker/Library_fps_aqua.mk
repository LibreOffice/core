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

$(eval $(call gb_Library_Library,fps_aqua))

$(eval $(call gb_Library_set_componentfile,fps_aqua,fpicker/source/aqua/fps_aqua))

$(eval $(call gb_Library_use_packages,fps_aqua,\
	vcl_inc \
))

$(eval $(call gb_Library_use_sdk_api,fps_aqua))

$(eval $(call gb_Library_add_libs,fps_aqua,\
    -framework Cocoa \
    -framework CoreFoundation \
))

$(eval $(call gb_Library_use_libraries,fps_aqua,\
	cppu \
	cppuhelper \
	sal \
	tl \
	vcl \
))

$(eval $(call gb_Library_add_standard_system_libs,fps_aqua))

$(eval $(call gb_Library_add_objcxxobjects,fps_aqua,\
	fpicker/source/aqua/AquaFilePickerDelegate \
	fpicker/source/aqua/CFStringUtilities \
	fpicker/source/aqua/ControlHelper \
	fpicker/source/aqua/FilterHelper \
	fpicker/source/aqua/FPentry \
	fpicker/source/aqua/NSString_OOoAdditions \
	fpicker/source/aqua/NSURL_OOoAdditions \
	fpicker/source/aqua/resourceprovider \
	fpicker/source/aqua/SalAquaFilePicker \
	fpicker/source/aqua/SalAquaFolderPicker \
	fpicker/source/aqua/SalAquaPicker \
))

# vim: set noet sw=4 ts=4:
