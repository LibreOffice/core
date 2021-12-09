# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,lnth))

$(eval $(call gb_Library_set_componentfile,lnth,lingucomponent/source/thesaurus/libnth/lnth,services))

$(eval $(call gb_Library_set_include,lnth,\
	$$(INCLUDE) \
	-I$(SRCDIR)/lingucomponent/source/lingutil \
))

$(eval $(call gb_Library_use_sdk_api,lnth))

$(eval $(call gb_Library_use_libraries,lnth,\
	comphelper \
	cppu \
	cppuhelper \
	i18nlangtag \
	lng \
	sal \
	tl \
	utl \
))

$(eval $(call gb_Library_use_static_libraries,lnth,\
	ulingu \
))

$(eval $(call gb_Library_use_externals,lnth,\
	boost_headers \
	hunspell \
	mythes \
))

$(eval $(call gb_Library_add_exception_objects,lnth,\
	lingucomponent/source/thesaurus/libnth/nthesdta \
	lingucomponent/source/thesaurus/libnth/nthesimp \
))

# vim: set noet sw=4 ts=4:
