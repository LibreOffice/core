# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,i18npool/localedata))

i18npool_LD_NAMES := $(basename $(notdir $(wildcard $(SRCDIR)/i18npool/source/localedata/data/*.xml)))

$(eval $(call gb_CustomTarget_register_targets,i18npool/localedata,\
	$(foreach name,$(i18npool_LD_NAMES),localedata_$(name).cxx) \
))

$(gb_CustomTarget_workdir)/i18npool/localedata/localedata_%_new.cxx : \
		$(SRCDIR)/i18npool/source/localedata/data/%.xml \
		$(call gb_Executable_get_runtime_dependencies,saxparser)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),SAX,1)
	$(call gb_Trace_StartRange,$(subst $(WORKDIR)/,,$@),SAX)
	$(call gb_Helper_abbreviate_dirs, \
		$(call gb_Helper_print_on_error, \
			$(call gb_Helper_execute,saxparser) $* $< $@ \
				-env:LO_LIB_DIR=$(call gb_Helper_make_url,$(INSTROOT_FOR_BUILD)/$(LIBO_LIB_FOLDER)) \
				-env:URE_MORE_SERVICES=$(call gb_Helper_make_url,$(call gb_Rdb_get_target_for_build,saxparser))))
	$(call gb_Trace_EndRange,$(subst $(WORKDIR)/,,$@),SAX)

.PRECIOUS: $(gb_CustomTarget_workdir)/i18npool/localedata/localedata_%_new.cxx
$(gb_CustomTarget_workdir)/i18npool/localedata/localedata_%.cxx : \
		$(gb_CustomTarget_workdir)/i18npool/localedata/localedata_%_new.cxx
	$(call gb_Helper_copy_if_different_and_touch,$<,$@)

# vim: set noet sw=4 ts=4:
