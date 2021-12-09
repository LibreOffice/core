# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,stringresource))

$(eval $(call gb_Library_set_componentfile,stringresource,scripting/source/stringresource/stringresource,services))

$(eval $(call gb_Library_set_include,stringresource,\
	$$(INCLUDE) \
	-I$(SRCDIR)/scripting/source/inc \
))

$(eval $(call gb_Library_use_external,stringresource,boost_headers))

$(eval $(call gb_Library_use_sdk_api,stringresource))

$(eval $(call gb_Library_use_libraries,stringresource,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	tl \
	i18nlangtag \
))

$(eval $(call gb_Library_add_exception_objects,stringresource,\
	scripting/source/stringresource/stringresource \
))

# vim: set noet sw=4 ts=4:
