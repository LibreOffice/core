# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# IMPORTANT NOTE: make sure, the "else" part works as expected. This normally
# means, that the condition ends with "$(1),$(2))" or the reverse. It should
# just end in two (!) braces, otherwise you may need to use either the $(1)
# or the $(2) multiple times.

define gb_CondCppunitMainLibOrExe
$(if $(or $(CROSS_COMPILING),$(DISABLE_DYNLOADING)),$(1),$(2))
endef

define gb_CondExeLockfile
$(if $(and $(filter-out ANDROID MACOSX iOS WNT,$(OS))),$(1),$(2))
endef

define gb_CondExeRegistryTools
$(if $(or $(DISABLE_DYNLOADING),$(ENABLE_MACOSX_SANDBOX)),$(2),$(1))
endef

define gb_CondExeSp2bv
$(if $(and $(filter WNT,$(OS)),$(call gb_Helper_optionals_and,DESKTOP ODK,$(true))),$(1),$(2))
endef

define gb_CondExeUno
$(if $(and $(filter DESKTOP,$(BUILD_TYPE)),$(if $(DISABLE_DYNLOADING),,$(true))),$(1),$(2))
endef

define gb_CondExeUnopkg
$(if $(and $(filter DESKTOP,$(BUILD_TYPE)),$(if $(DISABLE_DYNLOADING),,$(true))),$(1),$(2))
endef

define gb_CondLibSalTextenc
$(if $(or $(filter ANDROID,$(OS)),$(DISABLE_DYNLOADING)),$(2),$(1))
endef

# vim: set noet sw=4 ts=4:
