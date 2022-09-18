# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,cln,cln))

$(eval $(call gb_ExternalPackage_use_external_project,cln,cln))

ifeq ($(OS),MACOSX)
$(eval $(call gb_ExternalPackage_add_file,cln,$(LIBO_LIB_FOLDER)/libcln.dylib,src/.libs/libcln.dylib))
else ifeq ($(OS),WNT)
$(eval $(call gb_ExternalPackage_add_file,cln,$(LIBO_LIB_FOLDER)/libcln.lib,instdir/lib/libcln.lib))
else ifeq ($(DISABLE_DYNLOADING),)
$(eval $(call gb_ExternalPackage_add_file,cln,$(LIBO_LIB_FOLDER)/libcln.so.6,src/.libs/libcln.so.6.0.6))
endif

# vim: set noet sw=4 ts=4:
