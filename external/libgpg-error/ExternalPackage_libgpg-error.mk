# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,libgpg-error,libgpg-error))

$(eval $(call gb_ExternalPackage_use_external_project,libgpg-error,libgpg-error))

ifneq ($(DISABLE_DYNLOADING),TRUE)

ifeq ($(OS),LINUX)

$(eval $(call gb_ExternalPackage_add_file,libgpg-error,$(LIBO_LIB_FOLDER)/libgpg-error-lo.so.0,src/.libs/libgpg-error-lo.so.0.22.0))

else ifeq ($(OS),MACOSX)

$(eval $(call gb_ExternalPackage_add_file,libgpg-error,$(LIBO_LIB_FOLDER)/libgpg-error.0.dylib,src/.libs/libgpg-error.0.dylib))

endif

endif # $(DISABLE_DYNLOADING)

# vim: set noet sw=4 ts=4:
