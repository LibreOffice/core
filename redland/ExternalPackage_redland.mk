# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,redland,redland))

$(eval $(call gb_ExternalPackage_use_external_project,redland,redland))

ifeq ($(OS),MACOSX)
$(eval $(call gb_ExternalPackage_add_file,redland,lib/librdf-lo.$(REDLAND_MAJOR).dylib,src/.libs/librdf-lo.$(REDLAND_MAJOR).dylib))
$(eval $(call gb_ExternalPackage_add_file,redland,lib/librdf.dylib,src/.libs/librdf-lo.$(REDLAND_MAJOR).dylib))
else ifneq ($(filter IOS ANDROID,$(OS)),)
$(eval $(call gb_ExternalPackage_add_file,redland,lib/librdf.a,src/.libs/librdf.a))
else ifneq ($(filter WNTGCC,$(OS)$(COM)),)
$(eval $(call gb_ExternalPackage_add_file,redland,lib/librdf.dll.a,src/.libs/librdf.dll.a))
$(eval $(call gb_ExternalPackage_add_file,redland,bin/librdf-$(REDLAND_MAJOR).dll,src/.libs/librdf-$(REDLAND_MAJOR).dll))
else ifneq ($(filter WNT,$(OS)),)
$(eval $(call gb_ExternalPackage_add_file,redland,lib/librdf.a,src/.libs/librdf.a))
$(eval $(call gb_ExternalPackage_add_file,redland,bin/librdf.dll,src/.libs/librdf.dll))
else
$(eval $(call gb_ExternalPackage_add_file,redland,lib/librdf-lo.so.$(REDLAND_MAJOR),src/.libs/librdf-lo.so.$(REDLAND_MAJOR).0.0))
$(eval $(call gb_ExternalPackage_add_file,redland,lib/librdf.so,src/.libs/librdf-lo.so.$(REDLAND_MAJOR).0.0))
endif

# vim: set noet sw=4 ts=4:
