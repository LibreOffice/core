# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,coinmp_coinutils,coinmp))

$(eval $(call gb_ExternalPackage_use_external_project,coinmp_coinutils,coinmp))

ifeq ($(OS),WNT)
# The libraries are static.
else ifneq ($(DISABLE_DYNLOADING),)
# Just use the static archives from workdir. See bin/lo-all-static-libs
else ifeq ($(OS),MACOSX)
$(eval $(call gb_ExternalPackage_add_file,coinmp_coinutils,$(LIBO_LIB_FOLDER)/libCoinUtils.3.dylib,CoinUtils/src/.libs/libCoinUtils.3.11.13.dylib))
else
$(eval $(call gb_ExternalPackage_add_file,coinmp_coinutils,$(LIBO_LIB_FOLDER)/libCoinUtils.so.3,CoinUtils/src/.libs/libCoinUtils.so.3.11.13))
endif

# vim: set noet sw=4 ts=4:
