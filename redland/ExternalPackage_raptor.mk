# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,raptor,raptor))

$(eval $(call gb_ExternalPackage_use_external_project,raptor,raptor))

$(eval $(call gb_ExternalPackage_add_file,raptor,bin/raptor-config,src/raptor-config))

ifeq ($(OS),MACOSX)
$(eval $(call gb_ExternalPackage_add_file,raptor,lib/libraptor-lo.1.dylib,src/.libs/libraptor-lo.1.dylib))
$(eval $(call gb_ExternalPackage_add_file,raptor,lib/libraptor.dylib,src/.libs/libraptor-lo.1.dylib))
else ifneq ($(filter IOS ANDROID,$(OS)),)
$(eval $(call gb_ExternalPackage_add_file,raptor,lib/libraptor.a,src/.libs/libraptor.a))
else ifneq ($(filter WNTGCC,$(OS)$(COM)),)
$(eval $(call gb_ExternalPackage_add_file,raptor,lib/libraptor.dll.a,src/.libs/libraptor.dll.a))
$(eval $(call gb_ExternalPackage_add_file,raptor,bin/libraptor-1.dll,src/.libs/libraptor-1.dll))
else ifneq ($(filter WNT,$(OS)),)
$(eval $(call gb_ExternalPackage_add_file,raptor,lib/libraptor.a,src/.libs/libraptor.a))
$(eval $(call gb_ExternalPackage_add_file,raptor,bin/libraptor.dll,src/.libs/libraptor.dll))
else
$(eval $(call gb_ExternalPackage_add_file,raptor,lib/libraptor-lo.so.1,src/.libs/libraptor-lo.so.1.1.0))
$(eval $(call gb_ExternalPackage_add_file,raptor,lib/libraptor.so,src/.libs/libraptor-lo.so.1.1.0))
endif

$(eval $(call gb_ExternalPackage_add_file,raptor,inc/external/raptor.h,src/raptor.h))

# vim: set noet sw=4 ts=4:
