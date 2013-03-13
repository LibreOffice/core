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
$(eval $(call gb_ExternalPackage_add_file,redland,lib/librdf-lo.0.dylib,librdf/.libs/librdf-lo.0.dylib))
$(eval $(call gb_ExternalPackage_add_file,redland,lib/librdf.dylib,librdf/.libs/librdf-lo.0.dylib))
else ifneq ($(filter IOS ANDROID,$(OS)),)
$(eval $(call gb_ExternalPackage_add_file,redland,lib/librdf.a,librdf/.libs/librdf.a))
else ifneq ($(filter WNTGCC,$(OS)$(COM)),)
$(eval $(call gb_ExternalPackage_add_file,redland,lib/librdf.dll.a,librdf/.libs/librdf.dll.a))
$(eval $(call gb_ExternalPackage_add_file,redland,bin/librdf-0.dll,librdf/.libs/librdf-0.dll))
else ifneq ($(filter WNT,$(OS)),)
$(eval $(call gb_ExternalPackage_add_file,redland,lib/librdf.a,librdf/.libs/librdf.a))
$(eval $(call gb_ExternalPackage_add_file,redland,bin/librdf.dll,librdf/.libs/librdf.dll))
else
$(eval $(call gb_ExternalPackage_add_file,redland,lib/librdf-lo.so.0,librdf/.libs/librdf-lo.so.0.0.0))
$(eval $(call gb_ExternalPackage_add_file,redland,lib/librdf.so,librdf/.libs/librdf-lo.so.0.0.0))
endif

# vim: set noet sw=4 ts=4:
