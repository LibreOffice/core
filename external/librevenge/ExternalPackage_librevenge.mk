# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.$(REVENGE_RELEASE). If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.$(REVENGE_RELEASE)/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,librevenge,librevenge))

$(eval $(call gb_ExternalPackage_use_external_project,librevenge,librevenge))

ifeq ($(OS),MACOSX)
$(eval $(call gb_ExternalPackage_add_file,librevenge,$(LIBO_LIB_FOLDER)/librevenge-$(REVENGE_API_VERSION).dylib,src/lib/.libs/librevenge-$(REVENGE_API_VERSION).dylib))
else ifeq ($(OS),WNT)
$(eval $(call gb_ExternalPackage_add_file,librevenge,$(LIBO_LIB_FOLDER)/librevenge-$(REVENGE_API_VERSION).dll,src/lib/.libs/librevenge-$(REVENGE_API_VERSION).dll))
else ifeq ($(filter IOS ANDROID,$(OS)),)
$(eval $(call gb_ExternalPackage_add_file,librevenge,$(LIBO_LIB_FOLDER)/librevenge-$(REVENGE_API_VERSION).so.$(REVENGE_API_VERSION).$(REVENGE_RELEASE),src/lib/.libs/librevenge-$(REVENGE_API_VERSION).so.$(REVENGE_API_VERSION).$(REVENGE_RELEASE)))
endif

# vim: set noet sw=4 ts=4:
