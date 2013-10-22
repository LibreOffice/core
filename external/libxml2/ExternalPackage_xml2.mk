# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,xml2,xml2))

$(eval $(call gb_ExternalPackage_use_external_project,xml2,xml2))

ifneq ($(DISABLE_DYNLOADING),TRUE)
ifeq ($(OS),MACOSX)
$(eval $(call gb_ExternalPackage_add_library_for_install,xml2,lib/libxml2.2.dylib,.libs/libxml2.2.dylib,xml2))
else ifeq ($(OS),WNT)
ifeq ($(COM),GCC)
$(eval $(call gb_ExternalPackage_add_library_for_install,xml2,bin/libxml2.dll,.libs/libxml2.dll,xml2))
else # COM=MSC
$(eval $(call gb_ExternalPackage_add_library_for_install,xml2,bin/libxml2.dll,win32/bin.msvc/libxml2.dll,xml2))
endif
else # OS!=WNT
$(eval $(call gb_ExternalPackage_add_library_for_install,xml2,lib/libxml2.so.2,.libs/libxml2.so.2.7.6,xml2))
endif
endif # DISABLE_DYNLOADING

# vim: set noet sw=4 ts=4:
