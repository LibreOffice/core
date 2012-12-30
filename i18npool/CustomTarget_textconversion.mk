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

$(eval $(call gb_CustomTarget_CustomTarget,i18npool/textconversion))

$(call gb_CustomTarget_get_target,i18npool/textconversion) : \
	$(patsubst %.dic,$(call gb_CustomTarget_get_workdir,i18npool/textconversion)/%.cxx,$(notdir \
		$(wildcard $(SRCDIR)/i18npool/source/textconversion/data/*.dic)))

$(call gb_CustomTarget_get_workdir,i18npool/textconversion)/%.cxx : \
		$(SRCDIR)/i18npool/source/textconversion/data/%.dic \
		$(call gb_Executable_get_runtime_dependencies,genconv_dict) \
		| $(call gb_CustomTarget_get_workdir,i18npool/textconversion)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),CDC,1)
	$(call gb_Helper_abbreviate_dirs, \
		$(call gb_Helper_execute,genconv_dict) $* $< $@.tmp && \
		sed 's/\(^.*get\)/SAL_DLLPUBLIC_EXPORT \1/' $@.tmp > $@ && \
		rm $@.tmp)

# vim: set noet sw=4 ts=4:
