# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# for VERSION
include $(SRCDIR)/external/coinmp/version.mk

$(eval $(call gb_ExternalPackage_ExternalPackage,coinmp,coinmp))

$(eval $(call gb_ExternalPackage_use_external_project,coinmp,coinmp))

ifeq ($(OS),WNT)
$(eval $(call gb_ExternalPackage_add_file,coinmp,$(LIBO_LIB_FOLDER)/CoinMP.dll,CoinMP/MSVisualStudio/v9/$(if $(MSVC_USE_DEBUG_RUNTIME),Debug,Release)/CoinMP.dll))
else ifneq ($(DISABLE_DYNLOADING),)
# Just use the static archives from workdir. See bin/lo-all-static-libs
else ifeq ($(OS),MACOSX)
$(eval $(call gb_ExternalPackage_add_file,coinmp,$(LIBO_LIB_FOLDER)/libCbc.$(CBC_MAJOR_VERSION).dylib,Cbc/src/.libs/libCbc.$(CBC_VERSION).dylib))
$(eval $(call gb_ExternalPackage_add_file,coinmp,$(LIBO_LIB_FOLDER)/libCbcSolver.$(CBCSOLVER_MAJOR_VERSION).dylib,Cbc/src/.libs/libCbcSolver.$(CBCSOLVER_VERSION).dylib))
$(eval $(call gb_ExternalPackage_add_file,coinmp,$(LIBO_LIB_FOLDER)/libCgl.$(CGL_MAJOR_VERSION).dylib,Cgl/src/.libs/libCgl.$(CGL_VERSION).dylib))
$(eval $(call gb_ExternalPackage_add_file,coinmp,$(LIBO_LIB_FOLDER)/libClp.$(CLP_MAJOR_VERSION).dylib,Clp/src/.libs/libClp.$(CLP_VERSION).dylib))
$(eval $(call gb_ExternalPackage_add_file,coinmp,$(LIBO_LIB_FOLDER)/libOsiClp.$(OSICLP_MAJOR_VERSION).dylib,Clp/src/OsiClp/.libs/libOsiClp.$(OSICLP_VERSION).dylib))
$(eval $(call gb_ExternalPackage_add_file,coinmp,$(LIBO_LIB_FOLDER)/libCoinMP.$(COINMP_MAJOR_VERSION).dylib,CoinMP/src/.libs/libCoinMP.$(COINMP_VERSION).dylib))
$(eval $(call gb_ExternalPackage_add_file,coinmp,$(LIBO_LIB_FOLDER)/libCoinUtils.$(COINUTILS_MAJOR_VERSION).dylib,CoinUtils/src/.libs/libCoinUtils.$(COINUTILS_VERSION).dylib))
$(eval $(call gb_ExternalPackage_add_file,coinmp,$(LIBO_LIB_FOLDER)/libOsi.$(OSI_MAJOR_VERSION).dylib,Osi/src/Osi/.libs/libOsi.$(OSI_VERSION).dylib))
else
$(eval $(call gb_ExternalPackage_add_file,coinmp,$(LIBO_LIB_FOLDER)/libCbc.so.$(CBC_MAJOR_VERSION),Cbc/src/.libs/libCbc.so.$(CBC_VERSION)))
$(eval $(call gb_ExternalPackage_add_file,coinmp,$(LIBO_LIB_FOLDER)/libCbcSolver.so.$(CBCSOLVER_MAJOR_VERSION),Cbc/src/.libs/libCbcSolver.so.$(CBCSOLVER_VERSION)))
$(eval $(call gb_ExternalPackage_add_file,coinmp,$(LIBO_LIB_FOLDER)/libCgl.so.$(CGL_MAJOR_VERSION),Cgl/src/.libs/libCgl.so.$(CGL_VERSION)))
$(eval $(call gb_ExternalPackage_add_file,coinmp,$(LIBO_LIB_FOLDER)/libClp.so.$(CLP_MAJOR_VERSION),Clp/src/.libs/libClp.so.$(CLP_VERSION)))
$(eval $(call gb_ExternalPackage_add_file,coinmp,$(LIBO_LIB_FOLDER)/libOsiClp.so.$(OSICLP_MAJOR_VERSION),Clp/src/OsiClp/.libs/libOsiClp.so.$(OSICLP_VERSION)))
$(eval $(call gb_ExternalPackage_add_file,coinmp,$(LIBO_LIB_FOLDER)/libCoinMP.so.$(COINMP_MAJOR_VERSION),CoinMP/src/.libs/libCoinMP.so.$(COINMP_VERSION)))
$(eval $(call gb_ExternalPackage_add_file,coinmp,$(LIBO_LIB_FOLDER)/libCoinUtils.so.$(COINUTILS_MAJOR_VERSION),CoinUtils/src/.libs/libCoinUtils.so.$(COINUTILS_VERSION)))
$(eval $(call gb_ExternalPackage_add_file,coinmp,$(LIBO_LIB_FOLDER)/libOsi.so.$(OSI_MAJOR_VERSION),Osi/src/Osi/.libs/libOsi.so.$(OSI_VERSION)))
endif

# vim: set noet sw=4 ts=4:
