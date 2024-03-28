# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,libassuan,libassuan))

$(eval $(call gb_ExternalPackage_use_external_project,libassuan,libassuan))

ifneq ($(DISABLE_DYNLOADING),TRUE)

ifeq ($(OS),LINUX)

$(eval $(call gb_ExternalPackage_add_file,libassuan,$(LIBO_LIB_FOLDER)/libassuan.so.0,src/.libs/libassuan.so.0.8.7))

else ifeq ($(OS),MACOSX)

$(eval $(call gb_ExternalPackage_add_file,libassuan,$(LIBO_LIB_FOLDER)/libassuan.0.dylib,src/.libs/libassuan.0.dylib))

endif

endif # $(DISABLE_DYNLOADING)

# vim: set noet sw=4 ts=4:
