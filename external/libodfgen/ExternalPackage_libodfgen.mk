# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,libodfgen,libodfgen))

$(eval $(call gb_ExternalPackage_use_external_project,libodfgen,libodfgen))

ifeq ($(OS),MACOSX)
$(eval $(call gb_ExternalPackage_add_file,libodfgen,$(LIBO_LIB_FOLDER)/libodfgen-0.1.1.dylib,src/.libs/libodfgen-0.1.1.dylib))
else ifeq ($(OS),WNT)
$(eval $(call gb_ExternalPackage_add_file,libodfgen,$(LIBO_LIB_FOLDER)/libodfgen-0.1.dll,src/.libs/libodfgen-0.1.dll))
else ifeq ($(filter IOS ANDROID,$(OS)),)
$(eval $(call gb_ExternalPackage_add_file,libodfgen,$(LIBO_LIB_FOLDER)/libodfgen-0.1.so.1,src/.libs/libodfgen-0.1.so.1.0.3))
endif

# vim: set noet sw=4 ts=4:
