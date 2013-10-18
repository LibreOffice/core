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

ifneq ($(COM),MSC)
$(eval $(call gb_ExternalPackage_add_file,xml2,bin/xml2-config,xml2-config))
endif

ifeq ($(OS),MACOSX)
$(eval $(call gb_ExternalPackage_add_library_for_install,xml2,lib/libxml2.2.dylib,.libs/libxml2.2.dylib,xml2))
$(eval $(call gb_ExternalPackage_add_file,xml2,lib/libxml2.dylib,.libs/libxml2.2.dylib))
$(eval $(call gb_ExternalPackage_add_file,xml2,bin/xmllint,.libs/xmllint))
else ifeq ($(DISABLE_DYNLOADING),TRUE)
$(eval $(call gb_ExternalPackage_add_file,xml2,lib/libxml2.a,.libs/libxml2.a))
ifneq ($(CROSS_COMPILING),YES)
$(eval $(call gb_ExternalPackage_add_file,xml2,bin/xmllint,.libs/xmllint))
endif
else ifeq ($(OS),WNT)
ifeq ($(COM),GCC)
$(eval $(call gb_ExternalPackage_add_file,xml2,lib/libxml2.dll.a,.libs/libxml2.dll.a))
$(eval $(call gb_ExternalPackage_add_library_for_install,xml2,bin/libxml2.dll,.libs/libxml2.dll,xml2))
$(eval $(call gb_ExternalPackage_add_file,xml2,bin/xmllint.exe,.libs/xmllint.exe))
else # COM=MSC
$(eval $(call gb_ExternalPackage_add_file,xml2,lib/libxml2.lib,win32/bin.msvc/libxml2.lib))
$(eval $(call gb_ExternalPackage_add_library_for_install,xml2,bin/libxml2.dll,win32/bin.msvc/libxml2.dll,xml2))
$(eval $(call gb_ExternalPackage_add_file,xml2,bin/xmllint.exe,win32/bin.msvc/xmllint.exe))
endif
else # OS!=WNT
$(eval $(call gb_ExternalPackage_add_file,xml2,lib/libxml2.so.2.7.6,.libs/libxml2.so.2.7.6))
$(eval $(call gb_ExternalPackage_add_library_for_install,xml2,lib/libxml2.so.2,.libs/libxml2.so.2.7.6,xml2))
$(eval $(call gb_ExternalPackage_add_file,xml2,lib/libxml2.so,.libs/libxml2.so.2.7.6))
$(eval $(call gb_ExternalPackage_add_file,xml2,bin/xmllint,.libs/xmllint))
endif
# vim: set noet sw=4 ts=4:
