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

$(eval $(call gb_Library_Library,i18nlangtag))

$(eval $(call gb_Library_use_packages,i18nlangtag,\
	cppu_odk_headers \
	i18nlangtag_inc \
))

$(eval $(call gb_Library_set_include,i18nlangtag,\
	-I$(SRCDIR)/i18nlangtag/inc \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_use_sdk_api,i18nlangtag))

$(eval $(call gb_Library_add_defs,i18nlangtag,\
	-DI18NLANGTAG_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_use_libraries,i18nlangtag,\
	sal \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_exception_objects,i18nlangtag,\
	i18nlangtag/source/isolang/insys \
	i18nlangtag/source/isolang/isolang \
	i18nlangtag/source/isolang/mslangid \
	i18nlangtag/source/languagetag/languagetag \
))


ifeq ($(ENABLE_LIBLANGTAG),YES)
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
