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

$(eval $(call gb_Library_Library,dbp))

$(eval $(call gb_Library_set_componentfile,dbp,extensions/source/dbpilots/dbp,services))

$(eval $(call gb_Library_set_include,dbp,\
	$$(INCLUDE) \
	-I$(SRCDIR)/extensions/inc \
	-I$(SRCDIR)/extensions/source/inc \
))

$(eval $(call gb_Library_use_external,dbp,boost_headers))

$(eval $(call gb_Library_use_sdk_api,dbp))

$(eval $(call gb_Library_add_exception_objects,dbp,\
	extensions/source/dbpilots/commonpagesdbp \
	extensions/source/dbpilots/controlwizard \
	extensions/source/dbpilots/dbptools \
	extensions/source/dbpilots/gridwizard \
	extensions/source/dbpilots/groupboxwiz \
	extensions/source/dbpilots/listcombowizard \
	extensions/source/dbpilots/moduledbp \
	extensions/source/dbpilots/optiongrouplayouter \
	extensions/source/dbpilots/wizardservices \
))

$(eval $(call gb_Library_use_libraries,dbp,\
	svt \
	vcl \
	tl \
	tk \
	svl \
	sfx \
	dbtools \
	utl \
	comphelper \
	cppuhelper \
	cppu \
	sal \
	salhelper \
	i18nlangtag \
))

# vim:set noet sw=4 ts=4:
