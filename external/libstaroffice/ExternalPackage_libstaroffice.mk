# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,libstaroffice,libstaroffice))

$(eval $(call gb_ExternalPackage_use_external_project,libstaroffice,libstaroffice))

ifeq ($(OS),MACOSX)
$(eval $(call gb_ExternalPackage_add_file,libstaroffice,$(LIBO_LIB_FOLDER)/libstaroffice-0.0.0.dylib,src/lib/.libs/libstaroffice-0.0.0.dylib))
else ifeq ($(OS),WNT)
$(eval $(call gb_ExternalPackage_add_file,libstaroffice,$(LIBO_LIB_FOLDER)/libstaroffice-0.0.dll,src/lib/.libs/libstaroffice-0.0.dll))
else ifeq ($(DISABLE_DYNLOADING),)
ifeq ($(ANDROID_PORTS),1)
$(eval $(call gb_ExternalPackage_add_file,libstaroffice,$(LIBO_LIB_FOLDER)/libstaroffice-0.0-lo.so,src/lib/.libs/libstaroffice-0.0-lo.so))
else
$(eval $(call gb_ExternalPackage_add_file,libstaroffice,$(LIBO_LIB_FOLDER)/libstaroffice-0.0-lo.so.0,src/lib/.libs/libstaroffice-0.0-lo.so.0.0.$(STAROFFICE_VERSION_MICRO)))
endif
endif

# vim: set noet sw=4 ts=4:
