# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,ridljar/javamaker))

ridljar_DIR := $(gb_CustomTarget_workdir)/ridljar/javamaker

$(call gb_CustomTarget_get_target,ridljar/javamaker) : $(ridljar_DIR)/done

$(ridljar_DIR)/done : $(call gb_UnoApi_get_target,udkapi) \
		$(call gb_Executable_get_runtime_dependencies,javamaker) \
		| $(ridljar_DIR)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),JVM,1)
	$(call gb_Trace_StartRange,$(subst $(WORKDIR)/,,$@),JVM)
	$(call gb_Helper_abbreviate_dirs, \
	rm -r $(ridljar_DIR) && \
	$(call gb_Helper_execute,javamaker -O$(ridljar_DIR) $<) && touch $@)
	$(call gb_Trace_EndRange,$(subst $(WORKDIR)/,,$@),JVM)

# vim:set shiftwidth=4 tabstop=4 noexpandtab:
