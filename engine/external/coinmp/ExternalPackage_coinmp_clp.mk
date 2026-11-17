# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,coinmp_clp,coinmp))

$(eval $(call gb_ExternalPackage_use_external_project,coinmp_clp,coinmp))

ifeq ($(OS),WNT)
# The libraries are static.
else ifneq ($(DISABLE_DYNLOADING),)
# Just use the static archives from workdir. See bin/lo-all-static-libs
else ifeq ($(OS),MACOSX)
$(eval $(call gb_ExternalPackage_add_file,coinmp_clp,$(LIBO_LIB_FOLDER)/libClp.1.dylib,Clp/src/.libs/libClp.1.14.11.dylib))
$(eval $(call gb_ExternalPackage_add_file,coinmp_clp,$(LIBO_LIB_FOLDER)/libClpSolver.1.dylib,Clp/src/.libs/libClpSolver.1.14.11.dylib))
$(eval $(call gb_ExternalPackage_add_file,coinmp_clp,$(LIBO_LIB_FOLDER)/libOsiClp.1.dylib,Clp/src/OsiClp/.libs/libOsiClp.1.14.11.dylib))
else
$(eval $(call gb_ExternalPackage_add_file,coinmp_clp,$(LIBO_LIB_FOLDER)/libClp.so.1,Clp/src/.libs/libClp.so.1.14.11))
$(eval $(call gb_ExternalPackage_add_file,coinmp_clp,$(LIBO_LIB_FOLDER)/libClpSolver.so.1,Clp/src/.libs/libClpSolver.so.1.14.11))
$(eval $(call gb_ExternalPackage_add_file,coinmp_clp,$(LIBO_LIB_FOLDER)/libOsiClp.so.1,Clp/src/OsiClp/.libs/libOsiClp.so.1.14.11))
endif

# vim: set noet sw=4 ts=4:
