# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

$(eval $(call gb_CustomTarget_CustomTarget,net_ure/net_uretypes))

net_uretypes_DIR := $(gb_CustomTarget_workdir)/net_ure/net_uretypes

$(call gb_CustomTarget_get_target,net_ure/net_uretypes) : $(net_uretypes_DIR)/done

$(net_uretypes_DIR)/done : \
		$(call gb_UnoApi_get_target,udkapi) \
		$(call gb_Executable_get_target,netmaker) \
		$(call gb_Executable_get_runtime_dependencies,netmaker) \
		| $(net_uretypes_DIR)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),NET,4)
	$(call gb_Trace_StartRange,$(subst $(WORKDIR)/,,$@),NET)

	$(call gb_Helper_abbreviate_dirs,\
		$(call gb_Helper_print_on_error,\
			$(call gb_Helper_execute,\
				netmaker \
					-v \
					-O $(net_uretypes_DIR) \
					$(call gb_UnoApi_get_target,udkapi)) \
			$(file >$@,),\
			$(net_uretypes_DIR)/log))

	$(call gb_Trace_EndRange,$(subst $(WORKDIR)/,,$@),NET)

# vim:set shiftwidth=4 tabstop=4 noexpandtab:
