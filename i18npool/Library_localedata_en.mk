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

$(eval $(call gb_Library_Library,localedata_en))

$(eval $(call gb_Library_use_libraries,localedata_en,\
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_standard_system_libs,localedata_en))

$(eval $(call gb_Library_add_generated_exception_objects,localedata_en,\
	CustomTarget/i18npool/localedata/localedata_en_AU \
	CustomTarget/i18npool/localedata/localedata_en_BZ \
	CustomTarget/i18npool/localedata/localedata_en_CA \
	CustomTarget/i18npool/localedata/localedata_en_GB \
	CustomTarget/i18npool/localedata/localedata_en_GH \
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
