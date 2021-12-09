# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#

$(eval $(call gb_Library_Library,abp))

$(eval $(call gb_Library_set_componentfile,abp,extensions/source/abpilot/abp,services))

$(eval $(call gb_Library_set_include,abp,\
	$$(INCLUDE) \
	-I$(SRCDIR)/extensions/inc \
	-I$(SRCDIR)/extensions/source/inc \
))

$(eval $(call gb_Library_use_external,abp,boost_headers))

$(eval $(call gb_Library_use_sdk_api,abp))

$(eval $(call gb_Library_add_exception_objects,abp,\
	extensions/source/abpilot/abpfinalpage \
	extensions/source/abpilot/abspage \
	extensions/source/abpilot/abspilot \
	extensions/source/abpilot/admininvokationimpl \
	extensions/source/abpilot/admininvokationpage \
	extensions/source/abpilot/datasourcehandling \
	extensions/source/abpilot/fieldmappingimpl \
	extensions/source/abpilot/fieldmappingpage \
	extensions/source/abpilot/moduleabp \
	extensions/source/abpilot/tableselectionpage \
	extensions/source/abpilot/typeselectionpage \
	extensions/source/abpilot/unodialogabp \
))

$(eval $(call gb_Library_use_libraries,abp,\
	comphelper \
	cppuhelper \
	cppu \
	sal \
	sfx \
	svl \
	svt \
	svxcore \
	svx \
	tk \
	tl \
	utl \
	vcl \
	i18nlangtag \
))

# vim:set noet sw=4 ts=4:
