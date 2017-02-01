# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,lcms2,lcms2))

$(eval $(call gb_ExternalPackage_use_external_project,lcms2,lcms2))

ifeq ($(DISABLE_DYNLOADING),)
ifeq ($(OS),MACOSX)
$(eval $(call gb_ExternalPackage_add_file,lcms2,$(LIBO_LIB_FOLDER)/liblcms2.2.dylib,src/.libs/liblcms2.2.dylib))
else ifeq ($(OS),WNT)
ifeq ($(COM),GCC)
$(eval $(call gb_ExternalPackage_add_file,lcms2,$(LIBO_LIB_FOLDER)/liblcms2-2.dll,src/.libs/liblcms2-2.dll))
else ifeq ($(COM),MSC)
$(eval $(call gb_ExternalPackage_add_file,lcms2,$(LIBO_LIB_FOLDER)/lcms2.dll,bin/lcms2.dll))
endif # $(COM)
else  # $(OS) != WNT/MACOSX
$(eval $(call gb_ExternalPackage_add_file,lcms2,$(LIBO_LIB_FOLDER)/liblcms2.so.2,src/.libs/liblcms2.so.2.0.8))
endif # $(OS)
endif # $(DISABLE_DYNLOADING)

# vim: set noet sw=4 ts=4:
