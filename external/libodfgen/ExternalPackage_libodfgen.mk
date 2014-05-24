# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.$(ODFGEN_RELEASE). If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.$(ODFGEN_RELEASE)/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,libodfgen,libodfgen))

$(eval $(call gb_ExternalPackage_use_external_project,libodfgen,libodfgen))

ifeq ($(OS),MACOSX)
$(eval $(call gb_ExternalPackage_add_file,libodfgen,$(LIBO_LIB_FOLDER)/libodfgen-$(ODFGEN_API_VERSION).dylib,src/.libs/libodfgen-$(ODFGEN_API_VERSION).dylib))
else ifeq ($(OS),WNT)
ifeq ($(COM),GCC)
$(eval $(call gb_ExternalPackage_add_file,libodfgen,$(LIBO_LIB_FOLDER)/libodfgen-$(ODFGEN_API_VERSION).dll,src/.libs/libodfgen-$(ODFGEN_API_VERSION).dll))
else
$(eval $(call gb_ExternalPackage_add_file,libodfgen,$(LIBO_LIB_FOLDER)/libodfgen.dll,src/.libs/libodfgen.dll))
endif
else ifeq ($(filter IOS ANDROID,$(OS)),)
$(eval $(call gb_ExternalPackage_add_file,libodfgen,$(LIBO_LIB_FOLDER)/libodfgen-$(ODFGEN_API_VERSION).so.$(ODFGEN_API_VERSION).$(ODFGEN_RELEASE),src/.libs/libodfgen-$(ODFGEN_API_VERSION).so.$(ODFGEN_API_VERSION).$(ODFGEN_RELEASE)))
endif

# vim: set noet sw=4 ts=4:
