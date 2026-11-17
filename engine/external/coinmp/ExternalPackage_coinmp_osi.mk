# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,coinmp_osi,coinmp))

$(eval $(call gb_ExternalPackage_use_external_project,coinmp_osi,coinmp))

ifeq ($(OS),WNT)
# The libraries are static.
else ifneq ($(DISABLE_DYNLOADING),)
# Just use the static archives from workdir. See bin/lo-all-static-libs
else ifeq ($(OS),MACOSX)
$(eval $(call gb_ExternalPackage_add_file,coinmp_osi,$(LIBO_LIB_FOLDER)/libOsi.1.dylib,Osi/src/Osi/.libs/libOsi.1.13.12.dylib))
else
$(eval $(call gb_ExternalPackage_add_file,coinmp_osi,$(LIBO_LIB_FOLDER)/libOsi.so.1,Osi/src/Osi/.libs/libOsi.so.1.13.12))
endif

# vim: set noet sw=4 ts=4:
