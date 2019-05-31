# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,libqrgen,libqrgen))

$(eval $(call gb_ExternalPackage_use_external_project,libqrgen,libqrgen))

ifeq ($(OS),MACOSX)
$(eval $(call gb_ExternalPackage_add_file,libqrgen,$(LIBO_LIB_FOLDER)/libqrgen-0.1.1.dylib,src/.libs/libqrgen-0.1.1.dylib))
else ifeq ($(OS),WNT)
$(eval $(call gb_ExternalPackage_add_file,libqrgen,$(LIBO_LIB_FOLDER)/libqrgen-0.1.dll,src/.libs/libqrgen-0.1.dll))
else ifeq ($(DISABLE_DYNLOADING),)
$(eval $(call gb_ExternalPackage_add_file,libqrgen,$(LIBO_LIB_FOLDER)/libqrgen-0.1-lo.so.1,src/.libs/libqrgen-0.1-lo.so.1.0.$(QRGEN_VERSION_MICRO)))
endif

# vim: set noet sw=4 ts=4:
