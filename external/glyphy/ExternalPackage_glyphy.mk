# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,glyphy,glyphy))

$(eval $(call gb_ExternalPackage_use_external_project,glyphy,glyphy))

ifeq ($(OS),MACOSX)
$(eval $(call gb_ExternalPackage_add_file,glyphy,$(LIBO_LIB_FOLDER)/libglyphy.dylib,src/.libs/libglyphy.dylib))
else ifeq ($(OS),WNT)
# We build a static archive with MSVC, so nothing to add
else ifeq ($(DISABLE_DYNLOADING),)
$(eval $(call gb_ExternalPackage_add_file,glyphy,$(LIBO_LIB_FOLDER)/libglyphy.so.0,src/.libs/libglyphy.so.0.0.0))
endif

# vim: set noet sw=4 ts=4:
