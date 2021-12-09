# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,spell))

$(eval $(call gb_Library_set_componentfile,spell,lingucomponent/source/spellcheck/spell/spell,services))

$(eval $(call gb_Library_set_include,spell,\
	$$(INCLUDE) \
	-I$(SRCDIR)/lingucomponent/source/lingutil \
))

$(eval $(call gb_Library_use_sdk_api,spell))

$(eval $(call gb_Library_use_libraries,spell,\
	comphelper \
	cppu \
	cppuhelper \
	i18nlangtag \
	lng \
	sal \
	tl \
	utl \
))

$(eval $(call gb_Library_use_static_libraries,spell,\
	ulingu \
))

$(eval $(call gb_Library_use_externals,spell,\
	boost_headers \
	hunspell \
	icuuc \
))

$(eval $(call gb_Library_add_exception_objects,spell,\
	lingucomponent/source/spellcheck/spell/sspellimp \
))

# vim: set noet sw=4 ts=4:
