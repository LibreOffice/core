# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,liborcus,liborcus))

$(eval $(call gb_ExternalPackage_use_external_project,liborcus,liborcus))

ifeq ($(OS),MACOSX)
$(eval $(call gb_ExternalPackage_add_file,liborcus,$(LIBO_LIB_FOLDER)/liborcus-0.10.0.dylib,src/librocus/.libs/liborcus-0.10.0.dylib))
$(eval $(call gb_ExternalPackage_add_file,liborcus,$(LIBO_LIB_FOLDER)/liborcus-parser-0.10.0.dylib,src/parser/.libs/liborcus-parser-0.10.0.dylib))
else ifeq ($(OS),WNT)
$(eval $(call gb_ExternalPackage_add_file,liborcus,$(LIBO_LIB_FOLDER)/liborcus-0.10.dll,src/liborcus/.libs/liborcus-0.10.dll))
$(eval $(call gb_ExternalPackage_add_file,liborcus,$(LIBO_LIB_FOLDER)/liborcus-parser-0.10.dll,src/parser/.libs/liborcus-parser-0.10.dll))
else ifeq ($(filter IOS ANDROID,$(OS)),)
$(eval $(call gb_ExternalPackage_add_file,liborcus,$(LIBO_LIB_FOLDER)/liborcus-0.10.so.0,src/liborcus/.libs/liborcus-0.10.so.0))
$(eval $(call gb_ExternalPackage_add_file,liborcus,$(LIBO_LIB_FOLDER)/liborcus-parser-0.10.so.0,src/parser/.libs/liborcus-parser-0.10.so.0))
endif

# vim: set noet sw=4 ts=4:
