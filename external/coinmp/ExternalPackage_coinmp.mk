# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,coinmp,coinmp))

$(eval $(call gb_ExternalPackage_use_external_project,coinmp,coinmp))

ifeq ($(OS),WNT)
$(eval $(call gb_ExternalPackage_add_file,coinmp,$(LIBO_LIB_FOLDER)/CoinMP.dll,CoinMP/src/.libs/CoinMP.dll))
else ifeq ($(OS),MACOSX)
$(eval $(call gb_ExternalPackage_add_file,coinmp,$(LIBO_LIB_FOLDER)/libCoinMP.dylib,CoinMP/src/.libs/libCoinMP.1.dylib))
else
$(eval $(call gb_ExternalPackage_add_file,coinmp,$(LIBO_LIB_FOLDER)/libCoinMP.so,CoinMP/src/.libs/libCoinMP.so.1.7.6))
endif

# vim: set noet sw=4 ts=4:
