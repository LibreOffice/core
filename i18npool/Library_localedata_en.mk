# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,localedata_en))

$(eval $(call gb_Library_use_libraries,localedata_en,\
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_generated_exception_objects,localedata_en,\
	CustomTarget/i18npool/localedata/localedata_en_AU \
	CustomTarget/i18npool/localedata/localedata_en_BW \
	CustomTarget/i18npool/localedata/localedata_en_BZ \
	CustomTarget/i18npool/localedata/localedata_en_CA \
	CustomTarget/i18npool/localedata/localedata_en_GB \
	CustomTarget/i18npool/localedata/localedata_en_GH \
	CustomTarget/i18npool/localedata/localedata_en_GM \
	CustomTarget/i18npool/localedata/localedata_en_IE \
	CustomTarget/i18npool/localedata/localedata_en_JM \
	CustomTarget/i18npool/localedata/localedata_en_MW \
	CustomTarget/i18npool/localedata/localedata_en_NA \
	CustomTarget/i18npool/localedata/localedata_en_NZ \
	CustomTarget/i18npool/localedata/localedata_en_PH \
	CustomTarget/i18npool/localedata/localedata_en_TT \
	CustomTarget/i18npool/localedata/localedata_en_US \
	CustomTarget/i18npool/localedata/localedata_en_ZA \
	CustomTarget/i18npool/localedata/localedata_en_ZW \
))

# vim: set noet sw=4 ts=4:
