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

$(eval $(call gb_Module_Module,i18npool))

$(eval $(call gb_Module_add_targets,i18npool,\
	CustomTarget_breakiterator \
	CustomTarget_collator \
	CustomTarget_indexentry \
	CustomTarget_localedata \
	CustomTarget_textconversion \
	Library_collator_data \
	Library_dict_ja \
	Library_dict_zh \
	Library_i18nisolang1 \
	Library_i18npool \
	Library_i18nsearch \
	Library_index_data \
	Library_localedata_en \
	Library_localedata_es \
	Library_localedata_euro \
	Library_localedata_others \
	Library_textconv_dict \
	Package_inc \
))

ifneq ($(CROSS_COMPILING),YES)
$(eval $(call gb_Module_add_targets,i18npool,\
	Executable_gencoll_rule \
	Executable_genconv_dict \
	Executable_gendict \
	Executable_genindex_data \
	Executable_saxparser \
))
endif

$(eval $(call gb_Module_add_check_targets,i18npool,\
	CppunitTest_i18npool_test_breakiterator \
	CppunitTest_i18npool_test_characterclassification \
	CppunitTest_i18npool_test_languagetag \
))

# vim: set noet sw=4 ts=4:
