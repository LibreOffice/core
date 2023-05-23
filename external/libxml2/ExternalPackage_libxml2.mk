# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,libxml2,libxml2))

$(eval $(call gb_ExternalPackage_use_external_project,libxml2,libxml2))

ifneq ($(DISABLE_DYNLOADING),TRUE)
ifeq ($(OS),MACOSX)
$(eval $(call gb_ExternalPackage_add_file,libxml2,$(LIBO_URE_LIB_FOLDER)/libxml2.2.dylib,.libs/libxml2.2.dylib))
else ifeq ($(OS),WNT)
ifeq ($(COM),GCC)
$(eval $(call gb_ExternalPackage_add_file,libxml2,$(LIBO_URE_LIB_FOLDER)/libxml2.dll,.libs/libxml2.dll))
else # COM=MSC
$(eval $(call gb_ExternalPackage_add_file,libxml2,$(LIBO_URE_LIB_FOLDER)/libxml2.dll,win32/bin.msvc/libxml2.dll))
endif
else # OS!=WNT
$(eval $(call gb_ExternalPackage_add_file,libxml2,$(LIBO_URE_LIB_FOLDER)/libxml2.so.2,.libs/libxml2.so.2.11.$(LIBXML_VERSION_MICRO)))
endif
endif # DISABLE_DYNLOADING

# vim: set noet sw=4 ts=4:
