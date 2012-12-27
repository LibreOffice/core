# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,icu,icu))

$(eval $(call gb_ExternalPackage_use_external_project,icu,icu))
ifeq ($(OS),WNT)
ifeq ($(COM),GCC)

$(eval $(call gb_ExternalPackage_add_files,icu,bin,\
	source/lib/libicudt$(ICU_MAJOR)$(ICU_MINOR).dll \
	source/lib/libicuuc$(ICU_MAJOR)$(ICU_MINOR).dll \
	source/lib/libicuin$(ICU_MAJOR)$(ICU_MINOR).dll \
	source/lib/libicule$(ICU_MAJOR)$(ICU_MINOR).dll \
	source/lib/libicutu$(ICU_MAJOR)$(ICU_MINOR).dll \
	source/bin/genccode.exe \
	source/bin/genbrk.exe \
	source/bin/gencmn.exe \
))

else
$(eval $(call gb_ExternalPackage_add_files,icu,lib,\
	source/lib/icudt.lib \
	source/lib/icuuc.lib \
	source/lib/icuin.lib \
	source/lib/icule.lib \
	source/lib/icutu.lib\
))
$(eval $(call gb_ExternalPackage_add_files,icu,bin,\
	source/lib/icudt$(ICU_MAJOR).dll \
	source/lib/icuuc$(ICU_MAJOR).dll \
	source/lib/icuin$(ICU_MAJOR).dll \
	source/lib/icule$(ICU_MAJOR).dll \
	source/lib/icutu$(ICU_MAJOR).dll \
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
	source/lib/libicule.a \
	source/lib/libicutu.a \
))
else ifeq ($(OS),ANDROID)
$(eval $(call gb_ExternalPackage_add_files,icu,lib,\
	source/lib/libicudatalo.so \
	source/lib/libicuuclo.so \
	source/lib/libicui18nlo.so \
	source/lib/libiculelo.so \
	source/lib/libicutulo.so \
))
else
$(eval $(call gb_ExternalPackage_add_files,icu,lib,\
	source/lib/libicudata.$(gb_Library_DLLEXT).$(ICU_MAJOR).$(ICU_MINOR).$(ICU_MICRO) \
	source/lib/libicudata.$(gb_Library_DLLEXT).$(ICU_MAJOR) \
	source/lib/libicudata.$(gb_Library_DLLEXT) \
	source/lib/libicuuc.$(gb_Library_DLLEXT).$(ICU_MAJOR).$(ICU_MINOR).$(ICU_MICRO) \
	source/lib/libicuuc.$(gb_Library_DLLEXT).$(ICU_MAJOR) \
	source/lib/libicuuc.$(gb_Library_DLLEXT) \
	source/lib/libicui18n.$(gb_Library_DLLEXT).$(ICU_MAJOR).$(ICU_MINOR).$(ICU_MICRO) \
	source/lib/libicui18n.$(gb_Library_DLLEXT).$(ICU_MAJOR) \
	source/lib/libicui18n.$(gb_Library_DLLEXT) \
	source/lib/libicule.$(gb_Library_DLLEXT).$(ICU_MAJOR).$(ICU_MINOR).$(ICU_MICRO) \
	source/lib/libicule.$(gb_Library_DLLEXT).$(ICU_MAJOR) \
	source/lib/libicule.$(gb_Library_DLLEXT) \
	source/lib/libicutu.$(gb_Library_DLLEXT).$(ICU_MAJOR).$(ICU_MINOR).$(ICU_MICRO) \
	source/lib/libicutu.$(gb_Library_DLLEXT).$(ICU_MAJOR) \
	source/lib/libicutu.$(gb_Library_DLLEXT) \
))
$(eval $(call gb_ExternalPackage_add_files,icu,bin,\
	source/bin/genccode \
	source/bin/genbrk \
	source/bin/gencmn \
))
endif
endif
# vim: set noet sw=4 ts=4:
