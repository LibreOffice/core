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
	Library_i18npool \
	Library_i18nsearch \
	Library_localedata_en \
	Library_localedata_es \
	Library_localedata_euro \
	Library_localedata_others \
))

$(eval $(call gb_Module_add_targets_for_build,i18npool,\
	Executable_gencoll_rule \
	Executable_genconv_dict \
	Executable_genindex_data \
	Executable_saxparser \
	Rdb_saxparser \
))

$(eval $(call gb_Module_add_check_targets,i18npool,\
	CppunitTest_i18npool_breakiterator \
	CppunitTest_i18npool_characterclassification \
	CppunitTest_i18npool_ordinalsuffix \
	CppunitTest_i18npool_textsearch \
	CppunitTest_i18npool_calendar \
	CppunitTest_i18npool_defaultnumberingprovider \
	CppunitTest_i18npool_indexentry \
	CppunitTest_i18npool_transliteration \
))

# vim: set noet sw=4 ts=4:
