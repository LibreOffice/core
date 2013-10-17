# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

icu_VERSION := $(ICU_MAJOR).$(ICU_MINOR)$(if $(ICU_MICRO),.$(ICU_MICRO))

$(eval $(call gb_ExternalPackage_ExternalPackage,icu,icu))

$(eval $(call gb_ExternalPackage_use_external_project,icu,icu))
ifeq ($(OS),WNT)
ifeq ($(COM),GCC)

$(eval $(call gb_ExternalPackage_add_file,icu,lib/libicudata.dll.a,source/stubdata/icudt.lib))
$(eval $(call gb_ExternalPackage_add_file,icu,lib/libicuuc.dll.a,source/lib/icuuc.lib))
$(eval $(call gb_ExternalPackage_add_file,icu,lib/libicui18n.dll.a,source/lib/icuin.lib))
$(eval $(call gb_ExternalPackage_add_file,icu,lib/libicutu.dll.a,source/lib/icutu.lib))
$(eval $(call gb_ExternalPackage_add_libraries_for_install,icu,bin,\
	source/lib/icudt$(ICU_MAJOR).dll \
	source/lib/icuuc$(ICU_MAJOR).dll \
	source/lib/icuin$(ICU_MAJOR).dll \
	source/lib/icutu$(ICU_MAJOR).dll \
))
$(eval $(call gb_ExternalPackage_add_files,icu,bin,\
	source/bin/genccode.exe \
	source/bin/genbrk.exe \
	source/bin/gencmn.exe \
))

else
$(eval $(call gb_ExternalPackage_add_files,icu,lib,\
	source/lib/icudt$(if $(MSVC_USE_DEBUG_RUNTIME),d).lib \
	source/lib/icuuc$(if $(MSVC_USE_DEBUG_RUNTIME),d).lib \
	source/lib/icuin$(if $(MSVC_USE_DEBUG_RUNTIME),d).lib \
	source/lib/icutu$(if $(MSVC_USE_DEBUG_RUNTIME),d).lib\
))
$(eval $(call gb_ExternalPackage_add_libraries_for_install,icu,bin,\
	source/lib/icudt$(if $(MSVC_USE_DEBUG_RUNTIME),d)$(ICU_MAJOR).dll \
	source/lib/icuuc$(if $(MSVC_USE_DEBUG_RUNTIME),d)$(ICU_MAJOR).dll \
	source/lib/icuin$(if $(MSVC_USE_DEBUG_RUNTIME),d)$(ICU_MAJOR).dll \
	source/lib/icutu$(if $(MSVC_USE_DEBUG_RUNTIME),d)$(ICU_MAJOR).dll \
))
$(eval $(call gb_ExternalPackage_add_files,icu,bin,\
	source/bin/genccode.exe \
	source/bin/genbrk.exe \
	source/bin/gencmn.exe \
))
endif
else # $(OS)
ifeq ($(DISABLE_DYNLOADING),TRUE)
$(eval $(call gb_ExternalPackage_add_files,icu,lib,\
	source/lib/libicudata.a \
	source/lib/libicuuc.a \
	source/lib/libicui18n.a \
	source/lib/libicutu.a \
))
else ifeq ($(OS),ANDROID)
$(eval $(call gb_ExternalPackage_add_libraries_for_install,icu,lib,\
	source/lib/libicudatalo.so \
	source/lib/libicuuclo.so \
	source/lib/libicui18nlo.so \
	source/lib/libicutulo.so \
))
else
$(eval $(call gb_ExternalPackage_add_library_for_install,icu,lib/libicudata$(gb_Library_DLLEXT).$(ICU_MAJOR),source/lib/libicudata$(gb_Library_DLLEXT).$(icu_VERSION)))
$(eval $(call gb_ExternalPackage_add_file,icu,lib/libicudata$(gb_Library_DLLEXT),source/lib/libicudata$(gb_Library_DLLEXT).$(icu_VERSION)))
$(eval $(call gb_ExternalPackage_add_library_for_install,icu,lib/libicuuc$(gb_Library_DLLEXT).$(ICU_MAJOR),source/lib/libicuuc$(gb_Library_DLLEXT).$(icu_VERSION)))
$(eval $(call gb_ExternalPackage_add_file,icu,lib/libicuuc$(gb_Library_DLLEXT),source/lib/libicuuc$(gb_Library_DLLEXT).$(icu_VERSION)))
$(eval $(call gb_ExternalPackage_add_library_for_install,icu,lib/libicui18n$(gb_Library_DLLEXT).$(ICU_MAJOR),source/lib/libicui18n$(gb_Library_DLLEXT).$(icu_VERSION)))
$(eval $(call gb_ExternalPackage_add_file,icu,lib/libicui18n$(gb_Library_DLLEXT),source/lib/libicui18n$(gb_Library_DLLEXT).$(icu_VERSION)))
$(eval $(call gb_ExternalPackage_add_file,icu,lib/libicutu$(gb_Library_DLLEXT).$(ICU_MAJOR),source/lib/libicutu$(gb_Library_DLLEXT).$(icu_VERSION)))
$(eval $(call gb_ExternalPackage_add_file,icu,lib/libicutu$(gb_Library_DLLEXT),source/lib/libicutu$(gb_Library_DLLEXT).$(icu_VERSION)))

$(eval $(call gb_ExternalPackage_add_files,icu,bin,\
	source/bin/genccode \
	source/bin/genbrk \
	source/bin/gencmn \
))
endif
endif
# vim: set noet sw=4 ts=4:
