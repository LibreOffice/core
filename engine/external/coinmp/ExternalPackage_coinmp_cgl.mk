# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,coinmp_cgl,coinmp))

$(eval $(call gb_ExternalPackage_use_external_project,coinmp_cgl,coinmp))

ifeq ($(OS),WNT)
# The libraries are static.
else ifneq ($(DISABLE_DYNLOADING),)
# Just use the static archives from workdir. See bin/lo-all-static-libs
else ifeq ($(OS),MACOSX)
$(eval $(call gb_ExternalPackage_add_file,coinmp_cgl,$(LIBO_LIB_FOLDER)/libCgl.1.dylib,Cgl/src/.libs/libCgl.1.10.10.dylib))
else
$(eval $(call gb_ExternalPackage_add_file,coinmp_cgl,$(LIBO_LIB_FOLDER)/libCgl.so.1,Cgl/src/.libs/libCgl.so.1.10.10))
endif

# vim: set noet sw=4 ts=4:
