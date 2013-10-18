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

ifneq ($(COM),MSC)
$(eval $(call gb_ExternalPackage_add_file,xslt,bin/xslt-config,xslt-config))
endif

ifeq ($(OS),MACOSX)
$(eval $(call gb_ExternalPackage_add_library_for_install,xslt,lib/libxslt.1.dylib,libxslt/.libs/libxslt.1.dylib))
$(eval $(call gb_ExternalPackage_add_file,xslt,lib/libxslt.dylib,libxslt/.libs/libxslt.1.dylib))
$(eval $(call gb_ExternalPackage_add_library_for_install,xslt,lib/libexslt.0.dylib,libexslt/.libs/libexslt.0.dylib))
$(eval $(call gb_ExternalPackage_add_file,xslt,lib/libexslt.dylib,libexslt/.libs/libexslt.0.dylib))
$(eval $(call gb_ExternalPackage_add_file,xslt,bin/xsltproc,xsltproc/.libs/xsltproc))
else ifeq ($(DISABLE_DYNLOADING),TRUE)
$(eval $(call gb_ExternalPackage_add_file,xslt,lib/libxslt.a,libxslt/.libs/libxslt.a))
$(eval $(call gb_ExternalPackage_add_file,xslt,lib/libexslt.a,libexslt/.libs/libexslt.a))
ifneq ($(CROSS_COMPILING),YES)
$(eval $(call gb_ExternalPackage_add_file,xslt,bin/xsltproc,xsltproc/xsltproc))
endif
else ifeq ($(OS),WNT)
ifeq ($(COM),GCC)
$(eval $(call gb_ExternalPackage_add_file,xslt,lib/libxslt.dll.a,libxslt/.libs/libxslt.dll.a))
$(eval $(call gb_ExternalPackage_add_library_for_install,xslt,bin/libxslt.dll,libxslt/.libs/libxslt.dll))
$(eval $(call gb_ExternalPackage_add_file,xslt,lib/libexslt.dll.a,libexslt/.libs/libexslt.dll.a))
$(eval $(call gb_ExternalPackage_add_library_for_install,xslt,bin/libexslt.dll,libexslt/.libs/libexslt.dll))
$(eval $(call gb_ExternalPackage_add_file,xslt,bin/xsltproc.exe,xsltproc/.libs/xsltproc.exe))
else # COM=MSC
$(eval $(call gb_ExternalPackage_add_file,xslt,lib/libxslt.lib,win32/bin.msvc/libxslt.lib))
$(eval $(call gb_ExternalPackage_add_file,xslt,lib/libexslt.lib,win32/bin.msvc/libexslt.lib))
$(eval $(call gb_ExternalPackage_add_library_for_install,xslt,bin/libxslt.dll,win32/bin.msvc/libxslt.dll))
$(eval $(call gb_ExternalPackage_add_library_for_install,xslt,bin/libexslt.dll,win32/bin.msvc/libexslt.dll))
$(eval $(call gb_ExternalPackage_add_file,xslt,bin/xsltproc.exe,win32/bin.msvc/xsltproc.exe))
endif
else # OS!=WNT
$(eval $(call gb_ExternalPackage_add_file,xslt,lib/libxslt.so.1.1.26,libxslt/.libs/libxslt.so.1.1.26))
$(eval $(call gb_ExternalPackage_add_library_for_install,xslt,lib/libxslt.so.1,libxslt/.libs/libxslt.so.1.1.26))
$(eval $(call gb_ExternalPackage_add_file,xslt,lib/libxslt.so,libxslt/.libs/libxslt.so.1.1.26))
$(eval $(call gb_ExternalPackage_add_file,xslt,lib/libexslt.so.0.8.15,libexslt/.libs/libexslt.so.0.8.15))
$(eval $(call gb_ExternalPackage_add_library_for_install,xslt,lib/libexslt.so.0,libexslt/.libs/libexslt.so.0.8.15))
$(eval $(call gb_ExternalPackage_add_file,xslt,lib/libexslt.so,libexslt/.libs/libexslt.so.0.8.15))
$(eval $(call gb_ExternalPackage_add_file,xslt,bin/xsltproc,xsltproc/.libs/xsltproc))
endif
# vim: set noet sw=4 ts=4:
