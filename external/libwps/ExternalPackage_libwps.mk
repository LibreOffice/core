# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,libwps,libwps))

$(eval $(call gb_ExternalPackage_use_external_project,libwps,libwps))

ifeq ($(OS),MACOSX)
$(eval $(call gb_ExternalPackage_add_file,libwps,$(LIBO_LIB_FOLDER)/libwps-0.4.4.dylib,src/lib/.libs/libwps-0.4.4.dylib))
else ifeq ($(OS),WNT)
$(eval $(call gb_ExternalPackage_add_file,libwps,$(LIBO_LIB_FOLDER)/libwps-0.4.dll,src/lib/.libs/libwps-0.4.dll))
else ifeq ($(filter IOS ANDROID,$(OS)),)
$(eval $(call gb_ExternalPackage_add_file,libwps,$(LIBO_LIB_FOLDER)/libwps-0.4.so.4,src/lib/.libs/libwps-0.4.so.4.0.0))
endif

# vim: set noet sw=4 ts=4:
