# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,gpgmepp,gpgmepp))

$(eval $(call gb_ExternalPackage_use_external_project,gpgmepp,gpgmepp))

ifneq ($(DISABLE_DYNLOADING),TRUE)

ifeq ($(OS),LINUX)

$(eval $(call gb_ExternalPackage_add_file,gpgmepp,$(LIBO_LIB_FOLDER)/libgpgmepp.so.6,lang/cpp/src/.libs/libgpgmepp.so.6.20.1))
$(eval $(call gb_ExternalPackage_add_file,gpgmepp,$(LIBO_LIB_FOLDER)/libgpgme.so.11,src/.libs/libgpgme.so.11.32.1))

else ifeq ($(OS),MACOSX)

$(eval $(call gb_ExternalPackage_add_file,gpgmepp,$(LIBO_LIB_FOLDER)/libgpgmepp.6.dylib,lang/cpp/src/.libs/libgpgmepp.6.dylib))
$(eval $(call gb_ExternalPackage_add_file,gpgmepp,$(LIBO_LIB_FOLDER)/libgpgme.11.dylib,src/.libs/libgpgme.11.dylib))

else ifeq ($(OS),WNT)

$(eval $(call gb_ExternalPackage_add_file,gpgmepp,$(LIBO_LIB_FOLDER)/gpgme-w32spawn.exe,src/gpgme-w32spawn.exe))

endif

# If a tool executed during the build (like svidl) requires these gpgmepp libraries, it will also
# require those libassuan and libgpg-error libraries that these gpgmepp libraries link against:
$(call gb_Package_get_target_for_build,gpgmepp): \
     $(call gb_Helper_optional,LIBASSUAN,$(call gb_Package_get_target_for_build,libassuan)) \
     $(call gb_Helper_optional,LIBGPGERROR,$(call gb_Package_get_target_for_build,libgpg-error))

endif # $(DISABLE_DYNLOADING)

# vim: set noet sw=4 ts=4:
