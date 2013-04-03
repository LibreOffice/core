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

ifeq ($(OS),MACOSX)
$(eval $(call gb_ExternalPackage_add_file,raptor,lib/libraptor2-lo.$(RAPTOR_MAJOR).dylib,src/.libs/libraptor2-lo.$(RAPTOR_MAJOR).dylib))
$(eval $(call gb_ExternalPackage_add_file,raptor,lib/libraptor2.dylib,src/.libs/libraptor2-lo.$(RAPTOR_MAJOR).dylib))
else ifneq ($(filter IOS ANDROID,$(OS)),)
$(eval $(call gb_ExternalPackage_add_file,raptor,lib/libraptor2.a,src/.libs/libraptor2.a))
else ifneq ($(filter WNTGCC,$(OS)$(COM)),)
$(eval $(call gb_ExternalPackage_add_file,raptor,lib/libraptor2.dll.a,src/.libs/libraptor2.dll.a))
$(eval $(call gb_ExternalPackage_add_file,raptor,bin/libraptor2-$(RAPTOR_MAJOR).dll,src/.libs/libraptor2-$(RAPTOR_MAJOR).dll))
else ifneq ($(filter WNT,$(OS)),)
$(eval $(call gb_ExternalPackage_add_file,raptor,lib/libraptor2.a,src/.libs/libraptor2.a))
$(eval $(call gb_ExternalPackage_add_file,raptor,bin/libraptor2.dll,src/.libs/libraptor2.dll))
else
$(eval $(call gb_ExternalPackage_add_file,raptor,lib/libraptor2-lo.so.$(RAPTOR_MAJOR),src/.libs/libraptor2-lo.so.$(RAPTOR_MAJOR).0.0))
$(eval $(call gb_ExternalPackage_add_file,raptor,lib/libraptor2.so,src/.libs/libraptor2-lo.so.$(RAPTOR_MAJOR).0.0))
endif

# vim: set noet sw=4 ts=4:
