# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,glew,glew))

$(eval $(call gb_ExternalPackage_use_external_project,glew,glew))

ifeq ($(OS)-$(COM),WNT-MSC)
ifeq ($(CPUNAME),INTEL)
glew_arch_subdir=Win32
else ifeq ($(CPUNAME),X86_64)
glew_arch_subdir=x64
endif
endif

ifeq ($(OS),MACOSX)
$(eval $(call gb_ExternalPackage_add_file,glew,$(LIBO_LIB_FOLDER)/libGLEW.1.12.0.dylib,lib/libGLEW.1.12.0.dylib))
else ifeq ($(OS)-$(COM),WNT-GCC)
else ifeq ($(COM),MSC)
$(eval $(call gb_ExternalPackage_add_files,glew,$(LIBO_LIB_FOLDER), \
	bin/$(if $(MSVC_USE_DEBUG_RUNTIME),Debug/$(glew_arch_subdir)/glew32d.dll,Release/$(glew_arch_subdir)/glew32.dll) \
))
else ifeq ($(DISABLE_DYNLOADING),)
$(eval $(call gb_ExternalPackage_add_file,glew,$(LIBO_LIB_FOLDER)/libGLEW.so.1.12,lib/libGLEW.so.1.12.0))
endif

# vim: set noet sw=4 ts=4:
