# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

$(eval $(call gb_Library_Library,i18nlangtagicu))

$(eval $(call gb_Library_use_sdk_api,i18nlangtagicu))

$(eval $(call gb_Library_add_defs,i18nlangtagicu,\
	-DI18NLANGTAG_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_use_libraries,i18nlangtagicu,\
	sal \
	i18nlangtag \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_use_externals,i18nlangtagicu,\
	icu_headers \
	icuuc \
))

$(eval $(call gb_Library_add_exception_objects,i18nlangtagicu,\
	i18nlangtag/source/languagetag/languagetagicu \
))

# vim: set noet sw=4 ts=4:
