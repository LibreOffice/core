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

$(eval $(call gb_Library_Library,localedata_es))

$(eval $(call gb_Library_use_libraries,localedata_es,\
	localedata_en \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_standard_system_libs,localedata_es))

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
))

# vim: set noet sw=4 ts=4:
