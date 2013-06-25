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

ifeq ($(OS),MACOSX)
$(eval $(call gb_ExternalPackage_add_file,lcms2,lib/liblcms2.dylib,src/.libs/liblcms2.dylib))
$(eval $(call gb_ExternalPackage_add_library_for_install,lcms2,lib/liblcms2.2.dylib,src/.libs/liblcms2.2.dylib))
else ifeq ($(filter-out IOS ANDROID,$(OS)),)
$(eval $(call gb_ExternalPackage_add_file,lcms2,lib/liblcms2.a,src/.libs/liblcms2.a))
else ifeq ($(OS),WNT)
ifeq ($(COM),GCC)
$(eval $(call gb_ExternalPackage_add_file,lcms2,lib/liblcms2.dll.a,src/.libs/liblcms2.dll.a))
$(eval $(call gb_ExternalPackage_add_library_for_install,lcms2,bin/liblcms2-2.dll,src/.libs/liblcms2-2.dll))
else ifeq ($(COM),MSC)
$(eval $(call gb_ExternalPackage_add_file,lcms2,lib/lcms2$(if $(MSVC_USE_DEBUG_RUNTIME),d).lib,bin/lcms2.lib))
# note: the lcms2d.lib references LCMS2.DLL (without D!) but the dll is
# actually called LCMS2D.DLL then
$(eval $(call gb_ExternalPackage_add_library_for_install,lcms2,bin/lcms2.dll,bin/lcms2$(if $(MSVC_USE_DEBUG_RUNTIME),d).dll))
endif
else
$(eval $(call gb_ExternalPackage_add_file,lcms2,lib/liblcms2.so,src/.libs/liblcms2.so))
$(eval $(call gb_ExternalPackage_add_library_for_install,lcms2,lib/liblcms2.so.2,src/.libs/liblcms2.so.2.0.4))
endif
# vim: set noet sw=4 ts=4:
