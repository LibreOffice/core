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

define i18npool_LD_RULE
$(call gb_CustomTarget_get_workdir,i18npool/localedata)/localedata_$(1).cxx : \
		$(SRCDIR)/i18npool/source/localedata/data/$(1).xml \
		| $(call gb_Executable_get_runtime_dependencies,saxparser)
	$$(call gb_Output_announce,$$(subst $(WORKDIR)/,,$$@),$(true),SAX,1)
	$$(call gb_Helper_abbreviate_dirs, \
		$$(call gb_Helper_execute,saxparser) $(1) $$< $$@.tmp \
			-env:LO_LIB_DIR=$(call gb_Helper_make_url,$(gb_INSTROOT)/$(gb_PROGRAMDIRNAME)) \
			-env:URE_MORE_SERVICES=$(call gb_Helper_make_url,$(call gb_Rdb_get_target_for_build,saxparser)) \
			$(if $(findstring s,$(MAKEFLAGS)),> /dev/null 2>&1) && \
		sed 's/\(^.*get[^;]*$$$$\)/SAL_DLLPUBLIC_EXPORT \1/' $$@.tmp > $$@ && \
		rm $$@.tmp)

endef

$(foreach name,$(i18npool_LD_NAMES),$(eval $(call i18npool_LD_RULE,$(name))))

# vim: set noet sw=4 ts=4:
