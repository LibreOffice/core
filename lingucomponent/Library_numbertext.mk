# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,numbertext))

$(eval $(call gb_Library_set_componentfile,numbertext,lingucomponent/source/numbertext/numbertext,services))

$(eval $(call gb_Library_use_externals,numbertext,\
	libnumbertext \
))

$(eval $(call gb_Library_use_sdk_api,numbertext))

$(eval $(call gb_Library_use_libraries,numbertext,\
	cppu \
	cppuhelper \
	i18nlangtag \
	sal \
	tl \
	utl \
))

$(eval $(call gb_Library_add_exception_objects,numbertext,\
	lingucomponent/source/numbertext/numbertext \
))

# vim: set noet sw=4 ts=4:
