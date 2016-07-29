# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,curl,curl))

$(eval $(call gb_ExternalPackage_use_external_project,curl,curl))

ifneq ($(DISABLE_DYNLOADING),TRUE)

ifeq ($(OS)$(COM),WNTGCC)
$(eval $(call gb_ExternalPackage_add_file,curl,$(LIBO_LIB_FOLDER)/libcurl.dll,lib/.libs/libcurl.dll))
else ifeq ($(COM),MSC)
$(eval $(call gb_ExternalPackage_add_file,curl,$(LIBO_LIB_FOLDER)/libcurl$(if $(MSVC_USE_DEBUG_RUNTIME),d).dll,lib/$(if $(MSVC_USE_DEBUG_RUNTIME),debug-dll,release-dll)/libcurl$(if $(MSVC_USE_DEBUG_RUNTIME),d).dll))
else ifeq ($(OS),MACOSX)
$(eval $(call gb_ExternalPackage_add_file,curl,$(LIBO_LIB_FOLDER)/libcurl.4.dylib,lib/.libs/libcurl.4.dylib))
else ifeq ($(OS),AIX)
$(eval $(call gb_ExternalPackage_add_file,curl,$(LIBO_LIB_FOLDER)/libcurl.so,lib/.libs/libcurl.so.4))
else
$(eval $(call gb_ExternalPackage_add_file,curl,$(LIBO_LIB_FOLDER)/libcurl.so.4,lib/.libs/libcurl.so.4.4.0))
endif

endif # $(DISABLE_DYNLOADING)

# vim: set noet sw=4 ts=4:
