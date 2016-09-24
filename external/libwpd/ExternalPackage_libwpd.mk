# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,libwpd,libwpd))

$(eval $(call gb_ExternalPackage_use_external_project,libwpd,libwpd))

ifeq ($(OS),MACOSX)
$(eval $(call gb_ExternalPackage_add_file,libwpd,$(LIBO_LIB_FOLDER)/libwpd-0.10.10.dylib,src/lib/.libs/libwpd-0.10.10.dylib))
else ifeq ($(OS),WNT)
$(eval $(call gb_ExternalPackage_add_file,libwpd,$(LIBO_LIB_FOLDER)/libwpd-0.10.dll,src/lib/.libs/libwpd-0.10.dll))
else ifeq ($(DISABLE_DYNLOADING),)
$(eval $(call gb_ExternalPackage_add_file,libwpd,$(LIBO_LIB_FOLDER)/libwpd-0.10-lo.so.10,src/lib/.libs/libwpd-0.10-lo.so.10.0.$(WPD_VERSION_MICRO)))
endif

# vim: set noet sw=4 ts=4:
