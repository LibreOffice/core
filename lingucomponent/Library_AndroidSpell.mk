# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,AndroidSpell))

$(eval $(call gb_Library_set_componentfile,AndroidSpell,lingucomponent/source/spellcheck/androidspell/AndroidSpell))

$(eval $(call gb_Library_set_include,AndroidSpell,\
	$$(INCLUDE) \
	-I$(SRCDIR)/lingucomponent/source/lingutil \
))

$(eval $(call gb_Library_use_sdk_api,AndroidSpell))

$(eval $(call gb_Library_use_libraries,AndroidSpell,\
	comphelper \
	cppu \
	cppuhelper \
	i18nlangtag \
	lng \
	sal \
	tl \
	utl \
))

$(eval $(call gb_Library_use_static_libraries,AndroidSpell,\
	ulingu \
))

$(eval $(call gb_Library_use_externals,AndroidSpell,\
	boost_headers \
	icuuc \
))

$(eval $(call gb_Library_add_exception_objects,AndroidSpell,\
	lingucomponent/source/spellcheck/androidspell/androidspell \
))

# vim: set noet sw=4 ts=4:
