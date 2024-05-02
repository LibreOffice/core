# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,unoil/javamaker))

unoil_JAVADIR := $(gb_CustomTarget_workdir)/unoil/javamaker

$(call gb_CustomTarget_get_target,unoil/javamaker) : $(unoil_JAVADIR)/done

$(unoil_JAVADIR)/done : \
		$(call gb_UnoApi_get_target,offapi) \
		$(call gb_UnoApi_get_target,udkapi) \
		$(call gb_Executable_get_runtime_dependencies,javamaker) \
		| $(unoil_JAVADIR)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),JVM,1)
	$(call gb_Trace_StartRange,$(subst $(WORKDIR)/,,$@),JVM)
	$(call gb_Helper_abbreviate_dirs, \
	rm -r $(unoil_JAVADIR) && \
	$(call gb_Helper_execute,javamaker -O$(unoil_JAVADIR) \
		$(call gb_UnoApi_get_target,offapi) \
		-X$(call gb_UnoApi_get_target,udkapi)) && \
	touch $@)
	$(call gb_Trace_EndRange,$(subst $(WORKDIR)/,,$@),JVM)

# vim:set shiftwidth=4 tabstop=4 noexpandtab:
