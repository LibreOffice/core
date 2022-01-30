# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,localedata_es))

$(eval $(call gb_Library_set_plugin_for_nodep,localedata_es,i18npool))

$(eval $(call gb_Library_use_libraries,localedata_es,\
	localedata_en \
))

$(eval $(call gb_Library_add_generated_exception_objects,localedata_es,\
	CustomTarget/i18npool/localedata/localedata_es_AR \
	CustomTarget/i18npool/localedata/localedata_es_BO \
	CustomTarget/i18npool/localedata/localedata_es_CL \
	CustomTarget/i18npool/localedata/localedata_es_CO \
	CustomTarget/i18npool/localedata/localedata_es_CR \
	CustomTarget/i18npool/localedata/localedata_es_DO \
	CustomTarget/i18npool/localedata/localedata_es_EC \
	CustomTarget/i18npool/localedata/localedata_es_ES \
	CustomTarget/i18npool/localedata/localedata_es_GT \
	CustomTarget/i18npool/localedata/localedata_es_HN \
	CustomTarget/i18npool/localedata/localedata_es_MX \
	CustomTarget/i18npool/localedata/localedata_es_NI \
	CustomTarget/i18npool/localedata/localedata_es_PA \
	CustomTarget/i18npool/localedata/localedata_es_PE \
	CustomTarget/i18npool/localedata/localedata_es_PR \
	CustomTarget/i18npool/localedata/localedata_es_PY \
	CustomTarget/i18npool/localedata/localedata_es_SV \
	CustomTarget/i18npool/localedata/localedata_es_UY \
	CustomTarget/i18npool/localedata/localedata_es_VE \
	CustomTarget/i18npool/localedata/localedata_gl_ES \
	CustomTarget/i18npool/localedata/localedata_oc_ES \
))

# vim: set noet sw=4 ts=4:
