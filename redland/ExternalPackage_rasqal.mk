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

$(eval $(call gb_ExternalPackage_add_file,rasqal,bin/rasqal-config,src/rasqal-config))

ifeq ($(OS),MACOSX)
$(eval $(call gb_ExternalPackage_add_file,rasqal,lib/librasqal-lo.1.dylib,src/.libs/librasqal-lo.1.dylib))
$(eval $(call gb_ExternalPackage_add_file,rasqal,lib/librasqal.dylib,src/.libs/librasqal-lo.1.dylib))
else ifneq ($(filter IOS ANDRIOD,$(OS)),)
$(eval $(call gb_ExternalPackage_add_file,rasqal,lib/librasqal.a,src/.libs/librasqal.a))
else ifneq ($(filter WNT,$(OS)),)
$(eval $(call gb_ExternalPackage_add_file,rasqal,lib/librasqal.a,src/.libs/librasqal.a))
$(eval $(call gb_ExternalPackage_add_file,rasqal,bin/librasqal.dll,src/.libs/librasqal.dll))
else
$(eval $(call gb_ExternalPackage_add_file,rasqal,lib/librasqal-lo.so.1,src/.libs/librasqal-lo.so.1.0.0))
$(eval $(call gb_ExternalPackage_add_file,rasqal,lib/librasqal.so,src/.libs/librasqal-lo.so.1.0.0))
endif

# vim: set noet sw=4 ts=4:
