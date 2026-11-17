# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,coinmp_cbc,coinmp))

$(eval $(call gb_ExternalPackage_use_external_project,coinmp_cbc,coinmp))

ifeq ($(OS),WNT)
# The libraries are static.
else ifneq ($(DISABLE_DYNLOADING),)
# Just use the static archives from workdir. See bin/lo-all-static-libs
else ifeq ($(OS),MACOSX)
$(eval $(call gb_ExternalPackage_add_file,coinmp_cbc,$(LIBO_LIB_FOLDER)/libCbc.3.dylib,Cbc/src/.libs/libCbc.3.10.13.dylib))
$(eval $(call gb_ExternalPackage_add_file,coinmp_cbc,$(LIBO_LIB_FOLDER)/libCbcSolver.3.dylib,Cbc/src/.libs/libCbcSolver.3.10.13.dylib))
else
$(eval $(call gb_ExternalPackage_add_file,coinmp_cbc,$(LIBO_LIB_FOLDER)/libCbc.so.3,Cbc/src/.libs/libCbc.so.3.10.13))
$(eval $(call gb_ExternalPackage_add_file,coinmp_cbc,$(LIBO_LIB_FOLDER)/libCbcSolver.so.3,Cbc/src/.libs/libCbcSolver.so.3.10.13))
endif

# vim: set noet sw=4 ts=4:
