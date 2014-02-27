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
$(eval $(call gb_ExternalPackage_add_file,coinmp,$(LIBO_LIB_FOLDER)/CoinMP.dll,CoinMP/MSVisualStudio/v9/release/CoinMP.dll))
else ifeq ($(OS),MACOSX)
$(eval $(call gb_ExternalPackage_add_file,coinmp,$(LIBO_LIB_FOLDER)/libCbc.3.dylib,Cbc/src/.libs/libCbc.3.dylib))
$(eval $(call gb_ExternalPackage_add_file,coinmp,$(LIBO_LIB_FOLDER)/libCbcSolver.3.dylib,Cbc/src/.libs/libCbcSolver.3.dylib))
$(eval $(call gb_ExternalPackage_add_file,coinmp,$(LIBO_LIB_FOLDER)/libCgl.1.dylib,Cgl/src/.libs/libCgl.1.dylib))
$(eval $(call gb_ExternalPackage_add_file,coinmp,$(LIBO_LIB_FOLDER)/libClp.1.dylib,Clp/src/.libs/libClp.1.dylib))
$(eval $(call gb_ExternalPackage_add_file,coinmp,$(LIBO_LIB_FOLDER)/libOsiClp.1.dylib,Clp/src/OsiClp/.libs/libOsiClp.1.dylib))
$(eval $(call gb_ExternalPackage_add_file,coinmp,$(LIBO_LIB_FOLDER)/libCoinMP.1.dylib,CoinMP/src/.libs/libCoinMP.1.dylib))
$(eval $(call gb_ExternalPackage_add_file,coinmp,$(LIBO_LIB_FOLDER)/libCoinUtils.3.dylib,CoinUtils/src/.libs/libCoinUtils.3.dylib))
$(eval $(call gb_ExternalPackage_add_file,coinmp,$(LIBO_LIB_FOLDER)/libOsi.1.dylib,Osi/src/Osi/.libs/libOsi.1.dylib))
else
$(eval $(call gb_ExternalPackage_add_file,coinmp,$(LIBO_LIB_FOLDER)/libCbc.so.3,Cbc/src/.libs/libCbc.so.3.8.8))
$(eval $(call gb_ExternalPackage_add_file,coinmp,$(LIBO_LIB_FOLDER)/libCbcSolver.so.3,Cbc/src/.libs/libCbcSolver.so.3.8.8))
$(eval $(call gb_ExternalPackage_add_file,coinmp,$(LIBO_LIB_FOLDER)/libCgl.so.1,Cgl/src/.libs/libCgl.so.1.8.5))
$(eval $(call gb_ExternalPackage_add_file,coinmp,$(LIBO_LIB_FOLDER)/libClp.so.1,Clp/src/.libs/libClp.so.1.12.6))
$(eval $(call gb_ExternalPackage_add_file,coinmp,$(LIBO_LIB_FOLDER)/libOsiClp.so.1,Clp/src/OsiClp/.libs/libOsiClp.so.1.12.6))
$(eval $(call gb_ExternalPackage_add_file,coinmp,$(LIBO_LIB_FOLDER)/libCoinMP.so.1,CoinMP/src/.libs/libCoinMP.so.1.7.6))
$(eval $(call gb_ExternalPackage_add_file,coinmp,$(LIBO_LIB_FOLDER)/libCoinUtils.so.3,CoinUtils/src/.libs/libCoinUtils.so.3.9.11))
$(eval $(call gb_ExternalPackage_add_file,coinmp,$(LIBO_LIB_FOLDER)/libOsi.so.1,Osi/src/Osi/.libs/libOsi.so.1.11.5))
endif

# vim: set noet sw=4 ts=4:
