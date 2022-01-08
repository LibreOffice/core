# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

define gb_CondBuildUnopkg
$(if $(and $(filter DESKTOP,$(BUILD_TYPE)),$(if $(DISABLE_DYNLOADING),,$(true))),$(1),$(2))
endef

define gb_CondBuildLockfile
$(if $(and $(filter-out ANDROID MACOSX iOS WNT,$(OS))),$(1),$(2))
endef

define gb_CondBuildRegistryTools
$(if $(or $(DISABLE_DYNLOADING),$(ENABLE_MACOSX_SANDBOX)),$(2),$(1))
endef

define gb_CondExeSp2bv
$(if $(and $(filter WNT,$(OS)),$(call gb_Helper_optionals_and,DESKTOP ODK,$(true))),$(1),$(2))
endef

define gb_CondExeUno
$(if $(and $(filter DESKTOP,$(BUILD_TYPE)),$(if $(DISABLE_DYNLOADING),,$(true))),$(1),$(2))
endef

define gb_CondSalTextEncodingLibrary
$(if $(filter ANDROID,$(OS)),,$(if $(DISABLE_DYNLOADING),$(2),$(1)))
endef

# vim: set noet sw=4 ts=4:
