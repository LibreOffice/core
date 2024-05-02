# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,postprocess/check_dynamic_objects))

$(eval $(call gb_CustomTarget_register_targets,postprocess/check_dynamic_objects,\
	check.done \
))

$(gb_CustomTarget_workdir)/postprocess/check_dynamic_objects/check.done: \
		$(call gb_Postprocess_get_target,AllLibraries) \
		$(call gb_Postprocess_get_target,AllExecutables) \
		$(call gb_Postprocess_get_target,AllPackages)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),CHK,2)
	$(call gb_Trace_StartRange,$(subst $(WORKDIR)/,,$@),CHK)
	# currently we only check ELF objects
	$(SRCDIR)/bin/check-elf-dynamic-objects -p
	$(call gb_Trace_EndRange,$(subst $(WORKDIR)/,,$@),CHK)

# vim: set noet sw=4 ts=4:
