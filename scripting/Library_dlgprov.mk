# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,dlgprov))

$(eval $(call gb_Library_set_componentfile,dlgprov,scripting/source/dlgprov/dlgprov,services))

$(eval $(call gb_Library_set_include,dlgprov,\
	$$(INCLUDE) \
	-I$(SRCDIR)/scripting/source/inc \
))

$(eval $(call gb_Library_use_external,dlgprov,boost_headers))

$(eval $(call gb_Library_use_api,dlgprov,\
	offapi \
	oovbaapi \
	udkapi \
))

$(eval $(call gb_Library_use_libraries,dlgprov,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	i18nlangtag \
	sb \
	sfx \
	tl \
	ucbhelper \
	utl \
	vbahelper \
	vcl \
	xmlscript \
))

$(eval $(call gb_Library_add_exception_objects,dlgprov,\
	scripting/source/dlgprov/DialogModelProvider \
	scripting/source/dlgprov/dlgevtatt  \
	scripting/source/dlgprov/dlgprov \
))

# vim: set noet sw=4 ts=4:
