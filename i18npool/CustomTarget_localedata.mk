# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Initial Developer of the Original Code is
#       Matúš Kukan <matus.kukan@gmail.com>
# Portions created by the Initial Developer are Copyright (C) 2011 the
# Initial Developer. All Rights Reserved.
#
# Major Contributor(s):
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_CustomTarget_CustomTarget,i18npool/localedata))

i18npool_LDDIR := $(call gb_CustomTarget_get_workdir,i18npool/localedata)

$(call gb_CustomTarget_get_target,i18npool/localedata) : \
	$(patsubst %.xml,$(i18npool_LDDIR)/localedata_%.cxx, \
		$(notdir $(wildcard $(SRCDIR)/i18npool/source/localedata/data/*.xml)))

$(i18npool_LDDIR)/localedata_%.cxx : \
		$(SRCDIR)/i18npool/source/localedata/data/%.xml \
		$(i18npool_LDDIR)/saxparser.rdb \
		$(OUTDIR_FOR_BUILD)/bin/types.rdb \
		$(call gb_Executable_get_target_for_build,saxparser)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),SAX,1)
	$(call gb_Helper_abbreviate_dirs, \
		$(call gb_Helper_execute,saxparser) $* $< $@.tmp \
			$(call gb_Helper_make_url,$(i18npool_LDDIR)/saxparser.rdb) \
			$(call gb_Helper_make_url,$(OUTDIR_FOR_BUILD)/bin/types.rdb) \
			-env:LO_LIB_DIR=$(call gb_Helper_make_url,$(gb_Helper_OUTDIR_FOR_BUILDLIBDIR)) \
			$(if $(findstring s,$(MAKEFLAGS)),> /dev/null 2>&1) && \
		sed 's/\(^.*get[^;]*$$\)/SAL_DLLPUBLIC_EXPORT \1/' $@.tmp > $@ && \
		rm $@.tmp)

$(i18npool_LDDIR)/saxparser.rdb : $(i18npool_LDDIR)/saxparser.input \
		$(gb_XSLTPROCTARGET) $(SOLARENV)/bin/packcomponents.xslt
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),RDB,1)
	$(call gb_Helper_abbreviate_dirs, \
		$(gb_XSLTPROC) --nonet --stringparam prefix $(OUTDIR_FOR_BUILD)/xml/ \
			-o $@ $(SOLARENV)/bin/packcomponents.xslt $<)

$(i18npool_LDDIR)/saxparser.input : $(call gb_ComponentTarget_get_outdir_target,sax/source/expatwrap/expwrap) | $(i18npool_LDDIR)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ECH,1)
	echo '<list><filename>component/sax/source/expatwrap/expwrap.component</filename></list>' > $@

# vim: set noet sw=4 ts=4:
