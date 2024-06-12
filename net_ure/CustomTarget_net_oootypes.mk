# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

$(eval $(call gb_CustomTarget_CustomTarget,net_oootypes))

net_ure_DIR := $(gb_CustomTarget_workdir)/net_ure
net_oootypes_DIR := $(gb_CustomTarget_workdir)/net_ure/net_oootypes

$(call gb_CustomTarget_get_target,net_oootypes) : $(net_ure_DIR)/net_oootypes.done

$(net_ure_DIR)/net_oootypes.done : \
		$(call gb_UnoApi_get_target,offapi) \
		$(call gb_UnoApi_get_target,udkapi) \
		$(call gb_Executable_get_target,netmaker) \
		$(call gb_Executable_get_runtime_dependencies,netmaker) \
		| $(net_oootypes_DIR)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),NET,4)
	$(call gb_Trace_StartRange,$(subst $(WORKDIR)/,,$@),NET)
	$(call gb_Helper_abbreviate_dirs, \
	rm -r $(net_oootypes_DIR) && \
	$(call gb_Helper_execute,netmaker -v -O $(net_oootypes_DIR) \
		-X $(call gb_UnoApi_get_target,udkapi) \
		$(call gb_UnoApi_get_target,offapi) > $@.log 2>&1 || \
		(echo \
			&& cat $@.log \
			&& echo \
			&& echo "net_oootypes failed to generate. To retry, use:" \
			&& echo "    make CustomTarget_net_oootypes" \
			&& echo "cd into the net_ure/ directory to run make faster" \
			&& echo \
			&& false)) && \
	touch $@)
	$(call gb_Trace_EndRange,$(subst $(WORKDIR)/,,$@),NET)

# vim: set noet sw=4 ts=4:
