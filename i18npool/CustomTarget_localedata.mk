# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,i18npool/localedata))

i18npool_LDDIR := $(call gb_CustomTarget_get_workdir,i18npool/localedata)

i18npool_LD_NAMES := $(basename $(notdir $(wildcard $(SRCDIR)/i18npool/source/localedata/data/*.xml)))

$(call gb_CustomTarget_get_target,i18npool/localedata) : \
	$(foreach name,$(i18npool_LD_NAMES),$(i18npool_LDDIR)/localedata_$(name).cxx)

define i18npool_LD_RULE
$(i18npool_LDDIR)/localedata_$(1).cxx : \
		$(SRCDIR)/i18npool/source/localedata/data/$(1).xml \
		$(i18npool_LDDIR)/saxparser.rdb \
		| $(call gb_Executable_get_runtime_dependencies,saxparser)
	$$(call gb_Output_announce,$$(subst $(WORKDIR)/,,$$@),$(true),SAX,1)
	$$(call gb_Helper_abbreviate_dirs, \
		$(call gb_Helper_execute,saxparser) $(1) $$< $$@.tmp \
			$(call gb_Helper_make_url,$(i18npool_LDDIR)/saxparser.rdb) \
			-env:LO_LIB_DIR=$(call gb_Helper_make_url,$(gb_Helper_OUTDIR_FOR_BUILDLIBDIR) \
			-env:URE_MORE_SERVICES=$(call gb_Helper_make_url,$(i18npool_LDDIR)/saxparser.rdb)) \
			$(if $(findstring s,$(MAKEFLAGS)),> /dev/null 2>&1) && \
		sed 's/\(^.*get[^;]*$$$$\)/SAL_DLLPUBLIC_EXPORT \1/' $$@.tmp > $$@ && \
		rm $$@.tmp)

endef

$(foreach name,$(i18npool_LD_NAMES),$(eval $(call i18npool_LD_RULE,$(name))))

$(i18npool_LDDIR)/saxparser.rdb : $(i18npool_LDDIR)/saxparser.input \
		$(SOLARENV)/bin/packcomponents.xslt \
	| $(call gb_ExternalExecutable_get_dependencies,xsltproc)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),RDB,1)
	$(call gb_Helper_abbreviate_dirs, \
		$(call gb_ExternalExecutable_get_command,xsltproc) --nonet --stringparam prefix $(OUTDIR_FOR_BUILD)/xml/ \
			-o $@ $(SOLARENV)/bin/packcomponents.xslt $<)

$(i18npool_LDDIR)/saxparser.input : $(call gb_ComponentTarget_get_outdir_target,sax/source/expatwrap/expwrap) | $(i18npool_LDDIR)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ECH,1)
	echo '<list><filename>component/sax/source/expatwrap/expwrap.component</filename></list>' > $@

# vim: set noet sw=4 ts=4:
