# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,xml2_win32,xml2))

$(eval $(call gb_ExternalPackage_use_external_project,xml2_win32,xml2))

# duplicate copy next to executables due to stupid Win32 DLL search order
# where a libxml2.dll in C:\Windows is loaded before one in URE lib dir
# see 5a5dab5167d136c315e81448ea8eca59e60067da

ifeq ($(OS),WNT)
ifeq ($(COM),GCC)
$(eval $(call gb_ExternalPackage_add_file,xml2_win32,$(LIBO_LIB_FOLDER)/libxml2.dll,.libs/libxml2.dll))
else # COM=MSC
$(eval $(call gb_ExternalPackage_add_file,xml2_win32,$(LIBO_LIB_FOLDER)/libxml2.dll,win32/bin.msvc/libxml2.dll))
endif
endif # OS=WNT

# vim: set noet sw=4 ts=4:
