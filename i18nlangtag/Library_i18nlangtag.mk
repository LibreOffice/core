# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,i18nlangtag))

$(eval $(call gb_Library_use_sdk_api,i18nlangtag))

$(eval $(call gb_Library_add_defs,i18nlangtag,\
	-DI18NLANGTAG_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_use_libraries,i18nlangtag,\
	sal \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_use_externals,i18nlangtag,\
	boost_headers \
	icu_headers \
	icuuc \
))

$(eval $(call gb_Library_add_exception_objects,i18nlangtag,\
	i18nlangtag/source/isolang/insys \
	i18nlangtag/source/isolang/isolang \
	i18nlangtag/source/isolang/mslangid \
	i18nlangtag/source/languagetag/languagetag \
	i18nlangtag/source/languagetag/languagetagicu \
))


ifeq ($(ENABLE_LIBLANGTAG),TRUE)
$(eval $(call gb_Library_add_defs,i18nlangtag,-DENABLE_LIBLANGTAG))
$(eval $(call gb_Library_use_external,i18nlangtag,liblangtag))
$(eval $(call gb_Library_use_external,i18nlangtag,libxml2))
$(eval $(call gb_Library_use_system_win32_libs,i18nlangtag,\
        $(if $(filter $(COM),MSC), \
                kernel32 \
        ) \
))
endif

# vim: set noet sw=4 ts=4:
