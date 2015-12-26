# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,libwpg,libwpg))

$(eval $(call gb_ExternalPackage_use_external_project,libwpg,libwpg))

ifeq ($(OS),MACOSX)
$(eval $(call gb_ExternalPackage_add_file,libwpg,$(LIBO_LIB_FOLDER)/libwpg-0.3.3.dylib,src/lib/.libs/libwpg-0.3.3.dylib))
else ifeq ($(OS),WNT)
$(eval $(call gb_ExternalPackage_add_file,libwpg,$(LIBO_LIB_FOLDER)/libwpg-0.3.dll,src/lib/.libs/libwpg-0.3.dll))
else ifeq ($(DISABLE_DYNLOADING),)
$(eval $(call gb_ExternalPackage_add_file,libwpg,$(LIBO_LIB_FOLDER)/libwpg-0.3-lo.so.3,src/lib/.libs/libwpg-0.3-lo.so.3.0.$(WPG_VERSION_MICRO)))
endif

# vim: set noet sw=4 ts=4:
