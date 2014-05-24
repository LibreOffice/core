# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.$(ODFGEN_MICRO). If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.$(ODFGEN_MICRO)/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,libodfgen,libodfgen))

$(eval $(call gb_ExternalPackage_use_external_project,libodfgen,libodfgen))

ifeq ($(OS),MACOSX)
$(eval $(call gb_ExternalPackage_add_file,libodfgen,$(LIBO_LIB_FOLDER)/libodfgen-$(ODFGEN_MAJOR).$(ODFGEN_MINOR).$(ODFGEN_MICRO).dylib,src/.libs/libodfgen-$(ODFGEN_MAJOR).$(ODFGEN_MINOR).dylib))
else ifeq ($(OS),WNT)
ifeq ($(COM),GCC)
$(eval $(call gb_ExternalPackage_add_file,libodfgen,$(LIBO_LIB_FOLDER)/libodfgen-$(ODFGEN_MAJOR).$(ODFGEN_MINOR).dll,src/.libs/libodfgen-$(ODFGEN_MAJOR).$(ODFGEN_MINOR).dll))
else
$(eval $(call gb_ExternalPackage_add_file,libodfgen,$(LIBO_LIB_FOLDER)/libodfgen.dll,src/.libs/libodfgen.dll))
endif
else ifeq ($(filter IOS ANDROID,$(OS)),)
$(eval $(call gb_ExternalPackage_add_file,libodfgen,$(LIBO_LIB_FOLDER)/libodfgen-$(ODFGEN_MAJOR).$(ODFGEN_MINOR).so.$(ODFGEN_MAJOR),src/.libs/libodfgen-$(ODFGEN_MAJOR).$(ODFGEN_MINOR).so.$(ODFGEN_MAJOR).$(ODFGEN_MINOR).$(ODFGEN_MICRO)))
endif

# vim: set noet sw=4 ts=4:
