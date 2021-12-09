# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,hyphen))

$(eval $(call gb_Library_set_componentfile,hyphen,lingucomponent/source/hyphenator/hyphen/hyphen,services))

$(eval $(call gb_Library_set_include,hyphen,\
	$$(INCLUDE) \
	-I$(SRCDIR)/lingucomponent/source/lingutil \
))

$(eval $(call gb_Library_use_sdk_api,hyphen))

$(eval $(call gb_Library_use_libraries,hyphen,\
	comphelper \
	cppu \
	cppuhelper \
	i18nlangtag \
	lng \
	sal \
	tl \
	utl \
))

$(eval $(call gb_Library_use_static_libraries,hyphen,\
	ulingu \
))

$(eval $(call gb_Library_use_externals,hyphen,\
	boost_headers \
	hunspell \
	hyphen \
))

$(eval $(call gb_Library_add_exception_objects,hyphen,\
	lingucomponent/source/hyphenator/hyphen/hyphenimp \
))

# vim: set noet sw=4 ts=4:
