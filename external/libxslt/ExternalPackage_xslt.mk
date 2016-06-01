# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,xslt,xslt))

$(eval $(call gb_ExternalPackage_use_external_project,xslt,xslt))

ifneq ($(DISABLE_DYNLOADING),TRUE)
ifeq ($(OS),MACOSX)
$(eval $(call gb_ExternalPackage_add_file,xslt,$(LIBO_LIB_FOLDER)/libxslt.1.dylib,libxslt/.libs/libxslt.1.dylib))
$(eval $(call gb_ExternalPackage_add_file,xslt,$(LIBO_LIB_FOLDER)/libexslt.0.dylib,libexslt/.libs/libexslt.0.dylib))
else ifeq ($(OS),WNT)
ifeq ($(COM),GCC)
$(eval $(call gb_ExternalPackage_add_file,xslt,$(LIBO_LIB_FOLDER)/libxslt.dll,libxslt/.libs/libxslt.dll))
$(eval $(call gb_ExternalPackage_add_file,xslt,$(LIBO_LIB_FOLDER)/libexslt.dll,libexslt/.libs/libexslt.dll))
else # COM=MSC
$(eval $(call gb_ExternalPackage_add_file,xslt,$(LIBO_LIB_FOLDER)/libxslt.dll,win32/bin.msvc/libxslt.dll))
$(eval $(call gb_ExternalPackage_add_file,xslt,$(LIBO_LIB_FOLDER)/libexslt.dll,win32/bin.msvc/libexslt.dll))
endif
else # OS!=WNT
$(eval $(call gb_ExternalPackage_add_file,xslt,$(LIBO_LIB_FOLDER)/libxslt.so.1,libxslt/.libs/libxslt.so.1.1.29))
$(eval $(call gb_ExternalPackage_add_file,xslt,$(LIBO_LIB_FOLDER)/libexslt.so.0,libexslt/.libs/libexslt.so.0.8.17))
endif
endif # DISABLE_DYNLOADING

# vim: set noet sw=4 ts=4:
