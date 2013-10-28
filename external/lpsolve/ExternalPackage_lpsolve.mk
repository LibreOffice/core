# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,lpsolve,lpsolve))

$(eval $(call gb_ExternalPackage_use_external_project,lpsolve,lpsolve))

ifneq ($(DISABLE_DYNLOADING),TRUE)
ifeq ($(OS),WNT)
$(eval $(call gb_ExternalPackage_add_file,lpsolve,$(LIBO_LIB_FOLDER)/lpsolve55.dll,lpsolve55/lpsolve55.dll))
else ifeq ($(OS),MACOSX)
$(eval $(call gb_ExternalPackage_add_file,lpsolve,$(LIBO_LIB_FOLDER)/liblpsolve55.dylib,lpsolve55/liblpsolve55.dylib))
else # $(OS) != WNT/MACOSX
$(eval $(call gb_ExternalPackage_add_file,lpsolve,$(LIBO_LIB_FOLDER)/liblpsolve55.so,lpsolve55/liblpsolve55.so))
endif # $(OS)
endif # $(DISABLE_DYNLOADING)

# vim: set noet sw=4 ts=4:
