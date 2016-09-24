# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,librevenge,librevenge))

$(eval $(call gb_ExternalPackage_use_external_project,librevenge,librevenge))

ifeq ($(OS),MACOSX)
$(eval $(call gb_ExternalPackage_add_file,librevenge,$(LIBO_LIB_FOLDER)/librevenge-0.0.0.dylib,src/lib/.libs/librevenge-0.0.0.dylib))
else ifeq ($(OS),WNT)
$(eval $(call gb_ExternalPackage_add_file,librevenge,$(LIBO_LIB_FOLDER)/librevenge-0.0.dll,src/lib/.libs/librevenge-0.0.dll))
else ifeq ($(DISABLE_DYNLOADING),)
$(eval $(call gb_ExternalPackage_add_file,librevenge,$(LIBO_LIB_FOLDER)/librevenge-0.0-lo.so.0,src/lib/.libs/librevenge-0.0-lo.so.0.0.$(REVENGE_VERSION_MICRO)))
endif

# vim: set noet sw=4 ts=4:
