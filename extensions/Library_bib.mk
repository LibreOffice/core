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

$(eval $(call gb_Library_Library,bib))

$(eval $(call gb_Library_set_componentfile,bib,extensions/source/bibliography/bib,services))

$(eval $(call gb_Library_set_include,bib,\
	$$(INCLUDE) \
	-I$(SRCDIR)/extensions/inc \
))

$(eval $(call gb_Library_use_external,bib,boost_headers))

$(eval $(call gb_Library_use_sdk_api,bib))

$(eval $(call gb_Library_add_exception_objects,bib,\
	extensions/source/bibliography/bibbeam \
	extensions/source/bibliography/bibconfig \
	extensions/source/bibliography/bibcont \
	extensions/source/bibliography/bibload \
	extensions/source/bibliography/bibmod \
	extensions/source/bibliography/bibview \
	extensions/source/bibliography/datman \
	extensions/source/bibliography/formcontrolcontainer \
	extensions/source/bibliography/framectr \
	extensions/source/bibliography/general \
	extensions/source/bibliography/loadlisteneradapter \
	extensions/source/bibliography/toolbar \
))

$(eval $(call gb_Library_use_libraries,bib,\
	dbtools \
	sfx \
	sot \
	svt \
	svl \
	tk \
	vcl \
	utl \
	tl \
	comphelper \
	cppuhelper \
	cppu \
	sal \
	salhelper \
	i18nlangtag \
))

# vim:set noet sw=4 ts=4:
