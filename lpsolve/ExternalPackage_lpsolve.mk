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

ifeq ($(OS),WNT)
ifeq ($(COM),GCC)
ifneq ($(OS_FOR_BUILD),WNT)
$(eval $(call gb_ExternalPackage_add_files,lpsolve,lib,lpsolve55/liblpsolve55.dll.a))
endif # $(OS_FOR_BUILD)
else # $(COM)
$(eval $(call gb_ExternalPackage_add_files,lpsolve,lib,lpsolve55/lpsolve55.lib))
endif # $(COM)
$(eval $(call gb_ExternalPackage_add_library_for_install,lpsolve,bin/lpsolve55.dll,lpsolve55/lpsolve55.dll))
else # $(OS)

ifeq ($(OS),MACOSX)
$(eval $(call gb_ExternalPackage_add_library_for_install,lpsolve,lib/liblpsolve55.dylib,lpsolve55/liblpsolve55.dylib))
else
ifeq ($(DISABLE_DYNLOADING),TRUE)
$(eval $(call gb_ExternalPackage_add_files,lpsolve,lib,lpsolve55/liblpsolve55.a))
else
$(eval $(call gb_ExternalPackage_add_library_for_install,lpsolve,lib/liblpsolve55.so,lpsolve55/liblpsolve55.so))
endif # $(DISABLE_DYNLOADING)

endif # $(OS)
endif # $(OS)

# vim: set noet sw=4 ts=4:
