# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

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
	Library_i18npool \
	Library_i18nsearch \
	Library_index_data \
	Library_localedata_en \
	Library_localedata_es \
	Library_localedata_euro \
	Library_localedata_others \
	Library_textconv_dict \
))

$(eval $(call gb_Module_add_targets_for_build,i18npool,\
	Executable_gencoll_rule \
	Executable_genconv_dict \
	Executable_gendict \
	Executable_genindex_data \
	Executable_saxparser \
))

$(eval $(call gb_Module_add_check_targets,i18npool,\
	CppunitTest_i18npool_test_breakiterator \
	CppunitTest_i18npool_test_characterclassification \
	CppunitTest_i18npool_test_ordinalsuffix \
	CppunitTest_i18npool_test_textsearch \
))

# vim: set noet sw=4 ts=4:
