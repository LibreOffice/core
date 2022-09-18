# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,ginac,ginac))

$(eval $(call gb_ExternalPackage_use_external_project,ginac,ginac))

ifeq ($(OS),MACOSX)
$(eval $(call gb_ExternalPackage_add_file,ginac,$(LIBO_LIB_FOLDER)/libginac.dylib,ginac/.libs/libginac.dylib))
else ifeq ($(OS),WNT)
$(eval $(call gb_ExternalPackage_add_file,ginac,$(LIBO_LIB_FOLDER)/libginac.lib,ginac/.libs/libginac.lib))
else ifeq ($(DISABLE_DYNLOADING),)
$(eval $(call gb_ExternalPackage_add_file,ginac,$(LIBO_LIB_FOLDER)/libginac.so.11,ginac/.libs/libginac.so.11.1.1))
endif

# vim: set noet sw=4 ts=4:
