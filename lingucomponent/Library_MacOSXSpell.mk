# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,MacOSXSpell))

$(eval $(call gb_Library_set_componentfile,MacOSXSpell,lingucomponent/source/spellcheck/macosxspell/MacOSXSpell,services))

$(eval $(call gb_Library_set_include,MacOSXSpell,\
	$$(INCLUDE) \
	-I$(SRCDIR)/lingucomponent/source/lingutil \
))

$(eval $(call gb_Library_use_sdk_api,MacOSXSpell))

$(eval $(call gb_Library_use_libraries,MacOSXSpell,\
	comphelper \
	cppu \
	cppuhelper \
	lng \
	sal \
	i18nlangtag \
	svl \
	tl \
	ucbhelper \
	utl \
))

$(eval $(call gb_Library_use_system_darwin_frameworks,MacOSXSpell,\
	Cocoa \
))

$(eval $(call gb_Library_use_externals,MacOSXSpell,\
	boost_headers \
))

$(eval $(call gb_Library_add_objcxxobjects,MacOSXSpell,\
	lingucomponent/source/spellcheck/macosxspell/macspellimp \
))

# vim: set noet sw=4 ts=4:
