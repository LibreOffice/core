# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,i18npool/textconversion))

$(call gb_CustomTarget_get_target,i18npool/textconversion) : \
	$(patsubst %.dic,$(gb_CustomTarget_workdir)/i18npool/textconversion/%.cxx,$(notdir \
		$(wildcard $(SRCDIR)/i18npool/source/textconversion/data/*.dic)))

$(gb_CustomTarget_workdir)/i18npool/textconversion/%.cxx : \
		$(SRCDIR)/i18npool/source/textconversion/data/%.dic \
		$(call gb_Executable_get_runtime_dependencies,genconv_dict) \
		| $(gb_CustomTarget_workdir)/i18npool/textconversion/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),CDC,1)
	$(call gb_Trace_StartRange,$(subst $(WORKDIR)/,,$@),CDC)
	$(call gb_Helper_abbreviate_dirs, \
		$(call gb_Helper_execute,genconv_dict) $* $< $@ )
	$(call gb_Trace_EndRange,$(subst $(WORKDIR)/,,$@),CDC)

# vim: set noet sw=4 ts=4:
