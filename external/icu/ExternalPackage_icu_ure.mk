# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# libxml2 is in URE and depends on icuuc*.dll on Windows - extra package needed
$(eval $(call gb_ExternalPackage_ExternalPackage,icu_ure,icu))

$(eval $(call gb_ExternalPackage_use_external_project,icu_ure,icu))

ifneq ($(DISABLE_DYNLOADING),TRUE)
ifeq ($(OS),WNT)

ifeq ($(COM),GCC)
$(eval $(call gb_ExternalPackage_add_files,icu_ure,$(LIBO_URE_LIB_FOLDER),\
	source/lib/icudt$(ICU_MAJOR).dll \
	source/lib/icuuc$(ICU_MAJOR).dll \
))
else
$(eval $(call gb_ExternalPackage_add_files,icu_ure,$(LIBO_URE_LIB_FOLDER),\
	source/lib/icudt$(if $(MSVC_USE_DEBUG_RUNTIME),d)$(ICU_MAJOR).dll \
	source/lib/icuuc$(if $(MSVC_USE_DEBUG_RUNTIME),d)$(ICU_MAJOR).dll \
))
endif # $(COM)

endif # $(OS)
endif # DISABLE_DYNLOADING

# vim: set noet sw=4 ts=4:
