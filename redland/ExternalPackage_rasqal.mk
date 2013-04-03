# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,rasqal,rasqal))

$(eval $(call gb_ExternalPackage_use_external_project,rasqal,rasqal))

ifeq ($(OS),MACOSX)
$(eval $(call gb_ExternalPackage_add_file,rasqal,lib/librasqal-lo.$(RASQAL_MAJOR).dylib,src/.libs/librasqal-lo.$(RASQAL_MAJOR).dylib))
$(eval $(call gb_ExternalPackage_add_file,rasqal,lib/librasqal.dylib,src/.libs/librasqal-lo.$(RASQAL_MAJOR).dylib))
else ifneq ($(filter IOS ANDROID,$(OS)),)
$(eval $(call gb_ExternalPackage_add_file,rasqal,lib/librasqal.a,src/.libs/librasqal.a))
else ifneq ($(filter WNTGCC,$(OS)$(COM)),)
$(eval $(call gb_ExternalPackage_add_file,rasqal,lib/librasqal.dll.a,src/.libs/librasqal.dll.a))
$(eval $(call gb_ExternalPackage_add_file,rasqal,bin/librasqal-$(RASQAL_MAJOR).dll,src/.libs/librasqal-$(RASQAL_MAJOR).dll))
else ifneq ($(filter WNT,$(OS)),)
$(eval $(call gb_ExternalPackage_add_file,rasqal,lib/librasqal.a,src/.libs/librasqal.a))
$(eval $(call gb_ExternalPackage_add_file,rasqal,bin/librasqal.dll,src/.libs/librasqal.dll))
else
$(eval $(call gb_ExternalPackage_add_file,rasqal,lib/librasqal-lo.so.$(RASQAL_MAJOR),src/.libs/librasqal-lo.so.$(RASQAL_MAJOR).0.0))
$(eval $(call gb_ExternalPackage_add_file,rasqal,lib/librasqal.so,src/.libs/librasqal-lo.so.$(RASQAL_MAJOR).0.0))
endif

# vim: set noet sw=4 ts=4:
