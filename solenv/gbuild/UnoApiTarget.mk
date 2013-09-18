# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

# UnoApiTarget

gb_UnoApiTarget_UNOIDLWRITEDEPS := $(call gb_Executable_get_runtime_dependencies,unoidl-write)
gb_UnoApiTarget_UNOIDLWRITECOMMAND := SOLARBINDIR=$(gb_Executable_BINDIR_FOR_BUILD) $(call gb_Executable_get_command,unoidl-write)

gb_UnoApiTarget_UNOIDLCHECKDEPS := $(call gb_Executable_get_runtime_dependencies,unoidl-check)
gb_UnoApiTarget_UNOIDLCHECKCOMMAND := SOLARBINDIR=$(gb_Executable_BINDIR_FOR_BUILD) $(call gb_Executable_get_command,unoidl-check)

define gb_UnoApiTarget__command
$(call gb_Output_announce,$(2),$(true),UNO,4)
mkdir -p $(dir $(1)) \
$(if $(UNOAPI_ENTITIES), \
	&& RESPONSEFILE=$(call var2file,$(shell $(gb_MKTEMP)),500,$(UNOAPI_ENTITIES))) \
&& $(gb_UnoApiTarget_UNOIDLWRITECOMMAND) \
	$(foreach rdb,$(UNOAPI_DEPRDBS),$(call gb_UnoApiTarget_get_target,$(rdb))) \
	$(SRCDIR)/$(gb_UnoApiTarget_REG_$(2)) $(if $(UNOAPI_ENTITIES),@$${RESPONSEFILE}) $(1) \
$(if $(UNOAPI_ENTITIES),&& rm -f $${RESPONSEFILE}) \
$(if $(UNOAPI_REFERENCE), \
	$(call gb_Output_announce,$(2),$(true),DBc,3) \
	&& $(gb_UnoApiTarget_UNOIDLCHECKCOMMAND) $(UNOAPI_REFERENCE) -- \
		$(foreach rdb,$(UNOAPI_DEPRDBS),$(call gb_UnoApiTarget_get_target,$(rdb))) \
		$(1))
endef

$(call gb_UnoApiTarget_get_target,%) :
	$(call gb_UnoApiTarget__command,$@,$*)

.PHONY : $(call gb_UnoApiTarget_get_clean_target,%)
$(call gb_UnoApiTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),UNO,4)
	-$(call gb_Helper_abbreviate_dirs,\
		rm -rf $(call gb_UnoApiTarget_get_target,$*))

define gb_UnoApiTarget_UnoApiTarget
gb_UnoApiTarget_REG_$(1) := $(2)
$(call gb_UnoApiTarget_get_target,$(1)) : UNOAPI_ENTITIES :=
$(call gb_UnoApiTarget_get_target,$(1)) : UNOAPI_REFERENCE :=
$(call gb_UnoApiTarget_get_target,$(1)) : UNOAPI_DEPRDBS :=
$(call gb_UnoApiTarget_get_target,$(1)) : $(gb_UnoApiTarget_UNOIDLWRITEDEPS)
$(call gb_UnoApiTarget_get_target,$(1)) : $(SRCDIR)/$(2) # may be dir, though

endef

define gb_UnoApiTarget_add_idlfile
$(call gb_UnoApiTarget_get_target,$(1)) : UNOAPI_ENTITIES += $(subst /,.,$(2))$(if $(2),.)$(3)
$(call gb_UnoApiTarget_get_target,$(1)) : $(SRCDIR)/$(gb_UnoApiTarget_REG_$(1))/$(2)/$(3).idl

endef

define gb_UnoApiTarget_add_idlfiles
$(foreach idl,$(3),$(call gb_UnoApiTarget_add_idlfile,$(1),$(2),$(idl)))

endef

define gb_UnoApiTarget_set_reference_rdbfile
$(call gb_UnoApiTarget_get_target,$(1)) : UNOAPI_REFERENCE := $(2)
$(call gb_UnoApiTarget_get_target,$(1)) : $(2)
$(call gb_UnoApiTarget_get_target,$(1)) : $(gb_UnoApiTarget_UNOIDLCHECKDEPS)

endef

define gb_UnoApiTarget_use_api
$(call gb_UnoApiTarget_get_target,$(1)) : UNOAPI_DEPRDBS += $(2)
$(call gb_UnoApiTarget_get_target,$(1)) : $(call gb_UnoApiTarget_get_target,$(2))

endef

# UnoApiHeadersTarget

# defined by platform
#  gb_UnoApiHeadersTarget_select_variant

# Allow to redefine header variant.
#
# On iOS we use static linking because dynamic loading of own code
# isn't allowed by the iOS App Store rules, and we want our code to be
# eventually distributable there as part of apps.
#
# To avoid problems that this causes together with the lovely
# intentional breaking of the One Definition Rule, for iOS we always
# generate comprehensive headers for certain type RDBS. (The ODR
# breakage doesn't harm, by accident or careful design, on platforms
# where shared libraries are used.) To avoid generating the same headers
# more than once, we are silently "redirecting" the target to point to
# comprehensive headers instead.
#
# Example:
# If gb_UnoApiHeadersTarget_select_variant is defined as
#
# ifeq ($(DISABLE_DYNLOADING),TRUE)
# gb_UnoApiHeadersTarget_select_variant = $(if $(filter udkapi,$(1)),comprehensive,$(2))
# else
# gb_UnoApiHeadersTarget_select_variant = $(2)
# endif
#
# then, for the DISABLE_DYNLOADING case, whenever a makefile uses
# $(call gb_UnoApiHeadersTarget_get_target,udkapi) or $(call
# gb_UnoApiHeadersTarget_get_dir,udkapi), it will get target or dir for
# comprehensive headers instead.
#
# We are experimenting with static linking on Android, too. There for
# technical reasons to get around silly limitations in the OS, sigh.
#
# gb_UnoApiHeadersTarget_select_variant api default-variant
ifeq ($(origin gb_UnoApiHeadersTarget_select_variant),undefined)
$(eval $(call gb_Output_error,gb_UnoApiHeadersTarget_select_variant must be defined by platform))
endif

gb_UnoApiHeadersTarget_CPPUMAKERDEPS := $(call gb_Executable_get_runtime_dependencies,cppumaker)
gb_UnoApiHeadersTarget_CPPUMAKERCOMMAND := SOLARBINDIR=$(gb_Executable_BINDIR_FOR_BUILD) $(call gb_Executable_get_command,cppumaker)

define gb_UnoApiHeadersTarget__command
	$(gb_UnoApiHeadersTarget_CPPUMAKERCOMMAND) \
		-Gc $(4) -O$(3) $(call gb_UnoApiTarget_get_target,$(2)) \
		 $(UNOAPI_DEPS) && \
	touch $(1)

endef

$(call gb_UnoApiHeadersTarget_get_real_bootstrap_target,%) : \
		$(gb_UnoApiHeadersTarget_CPPUMAKERDEPS)
	$(call gb_Output_announce,$*,$(true),HPB,3) \
	$(call gb_UnoApiHeadersTarget__command,$@,$*,$(call gb_UnoApiHeadersTarget_get_bootstrap_dir,$*))

$(call gb_UnoApiHeadersTarget_get_real_comprehensive_target,%) : \
		$(gb_UnoApiHeadersTarget_CPPUMAKERDEPS)
	$(call gb_Output_announce,$*,$(true),HPC,3)
	$(call gb_UnoApiHeadersTarget__command,$@,$*,$(call gb_UnoApiHeadersTarget_get_comprehensive_dir,$*),-C)

$(call gb_UnoApiHeadersTarget_get_real_target,%) : \
		$(gb_UnoApiHeadersTarget_CPPUMAKERDEPS)
	$(call gb_Output_announce,$*,$(true),HPP,3) \
	$(call gb_UnoApiHeadersTarget__command,$@,$*,$(call gb_UnoApiHeadersTarget_get_dir,$*),-L)

.PHONY : $(call gb_UnoApiHeadersTarget_get_clean_target,%)
$(call gb_UnoApiHeadersTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),HPP,3)
	$(call gb_Helper_abbreviate_dirs,\
		rm -rf \
			$(call gb_UnoApiHeadersTarget_get_real_dir,$*) \
			$(call gb_UnoApiHeadersTarget_get_real_bootstrap_dir,$*) \
			$(call gb_UnoApiHeadersTarget_get_real_comprehensive_dir,$*) \
			$(call gb_UnoApiHeadersTarget_get_real_target,$*) \
			$(call gb_UnoApiHeadersTarget_get_real_bootstrap_target,$*)) \
			$(call gb_UnoApiHeadersTarget_get_real_comprehensive_target,$*)

define gb_UnoApiHeadersTarget_UnoApiHeadersTarget
$(call gb_UnoApiHeadersTarget_get_target,$(1)) : $(call gb_UnoApiTarget_get_target,$(1))
$(call gb_UnoApiHeadersTarget_get_bootstrap_target,$(1)) : $(call gb_UnoApiTarget_get_target,$(1))
$(call gb_UnoApiHeadersTarget_get_comprehensive_target,$(1)) : $(call gb_UnoApiTarget_get_target,$(1))
$(call gb_UnoApiHeadersTarget_get_clean_target,$(1)) : $(call gb_UnoApiTarget_get_clean_target,$(1))

$(call gb_UnoApiHeadersTarget_get_target,$(1)) : UNOAPI_DEPS :=
$(call gb_UnoApiHeadersTarget_get_bootstrap_target,$(1)) : UNOAPI_DEPS :=
$(call gb_UnoApiHeadersTarget_get_comprehensive_target,$(1)) : UNOAPI_DEPS :=

# need dummy recipes so that header files are delivered in Package_inc;
# otherwise make will consider the header to be up-to-date because it was
# actually built by the recipe for gb_UnoApiHeadersTarget_get_target
$(call gb_UnoApiHeadersTarget_get_real_dir,$(1))/%.hdl :| \
		$(call gb_UnoApiHeadersTarget_get_real_target,$(1))
	touch $$@

$(call gb_UnoApiHeadersTarget_get_real_dir,$(1))/%.hpp :| \
		$(call gb_UnoApiHeadersTarget_get_real_target,$(1))
	touch $$@

$(call gb_UnoApiHeadersTarget_get_real_bootstrap_dir,$(1))/%.hdl :| \
		$(call gb_UnoApiHeadersTarget_get_real_bootstrap_target,$(1))
	touch $$@

$(call gb_UnoApiHeadersTarget_get_real_bootstrap_dir,$(1))/%.hpp :| \
		$(call gb_UnoApiHeadersTarget_get_real_bootstrap_target,$(1))
	touch $$@

$(call gb_UnoApiHeadersTarget_get_real_comprehensive_dir,$(1))/%.hdl :| \
		$(call gb_UnoApiHeadersTarget_get_real_comprehensive_target,$(1))
	touch $$@

$(call gb_UnoApiHeadersTarget_get_real_comprehensive_dir,$(1))/%.hpp :| \
		$(call gb_UnoApiHeadersTarget_get_real_comprehensive_target,$(1))
	touch $$@

endef

define gb_UnoApiHeadersTarget__use_api_for_target
$(call gb_UnoApiHeadersTarget_get_$(3),$(1)) : $(call gb_UnoApiTarget_get_target,$(2))
$(call gb_UnoApiHeadersTarget_get_$(3),$(1)) : UNOAPI_DEPS += -X$(call gb_UnoApiTarget_get_target,$(2))

endef

define gb_UnoApiHeadersTarget_add_rdbfile
$$(call gb_Output_error,gb_UnoApiHeadersTarget_add_rdbfile: use gb_UnoApiHeadersTarget_use_api instead.)
endef

define gb_UnoApiHeadersTarget__use_api
$(call gb_UnoApiHeadersTarget__use_api_for_target,$(1),$(2),target)
$(call gb_UnoApiHeadersTarget__use_api_for_target,$(1),$(2),bootstrap_target)
$(call gb_UnoApiHeadersTarget__use_api_for_target,$(1),$(2),comprehensive_target)

endef

define gb_UnoApiHeadersTarget_add_rdbfiles
$$(call gb_Output_error,gb_UnoApiHeadersTarget_add_rdbfiles: use gb_UnoApiHeadersTarget_use_api instead.)
endef

define gb_UnoApiHeadersTarget_use_api
$(foreach rdb,$(2),$(call gb_UnoApiHeadersTarget__use_api,$(1),$(rdb)))

endef

# vim: set noet sw=4 ts=4:
