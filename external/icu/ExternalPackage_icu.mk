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

ifneq ($(DISABLE_DYNLOADING),TRUE)
ifeq ($(OS),WNT)

ifeq ($(COM),GCC)
$(eval $(call gb_ExternalPackage_add_files,icu,$(LIBO_LIB_FOLDER),\
	source/lib/icuin$(ICU_MAJOR).dll \
))
else
$(eval $(call gb_ExternalPackage_add_files,icu,$(LIBO_LIB_FOLDER),\
	source/lib/icuin$(if $(MSVC_USE_DEBUG_RUNTIME),d)$(ICU_MAJOR).dll \
))
endif # $(COM)

else ifeq ($(OS),ANDROID)

$(eval $(call gb_ExternalPackage_add_files,icu,$(LIBO_LIB_FOLDER),\
	source/lib/libicudatalo.so \
	source/lib/libicuuclo.so \
	source/lib/libicui18nlo.so \
))

else # $(OS) != WNT/ANDROID

$(eval $(call gb_ExternalPackage_add_file,icu,$(LIBO_LIB_FOLDER)/libicudata$(gb_Library_DLLEXT).$(ICU_MAJOR),source/lib/libicudata$(gb_Library_DLLEXT).$(icu_VERSION)))
$(eval $(call gb_ExternalPackage_add_file,icu,$(LIBO_LIB_FOLDER)/libicuuc$(gb_Library_DLLEXT).$(ICU_MAJOR),source/lib/libicuuc$(gb_Library_DLLEXT).$(icu_VERSION)))
$(eval $(call gb_ExternalPackage_add_file,icu,$(LIBO_LIB_FOLDER)/libicui18n$(gb_Library_DLLEXT).$(ICU_MAJOR),source/lib/libicui18n$(gb_Library_DLLEXT).$(icu_VERSION)))

endif # $(OS)
endif # DISABLE_DYNLOADING

# vim: set noet sw=4 ts=4:
