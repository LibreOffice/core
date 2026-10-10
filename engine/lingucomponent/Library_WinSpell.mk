# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# Copyright the Collabora Office contributors.
#
# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,WinSpell))

$(eval $(call gb_Library_set_componentfile,WinSpell,lingucomponent/source/spellcheck/winspell/WinSpell,services))

$(eval $(call gb_Library_set_include,WinSpell,\
	$$(INCLUDE) \
	-I$(SRCDIR)/lingucomponent/source/lingutil \
))

$(eval $(call gb_Library_use_sdk_api,WinSpell))

$(eval $(call gb_Library_use_libraries,WinSpell,\
	comphelper \
	cppu \
	cppuhelper \
	i18nlangtag \
	lng \
	sal \
	tl \
	utl \
))

$(eval $(call gb_Library_use_static_libraries,WinSpell,\
	ulingu \
))

$(eval $(call gb_Library_use_system_win32_libs,WinSpell,\
	ole32 \
))

$(eval $(call gb_Library_use_externals,WinSpell,\
	boost_headers \
))

$(eval $(call gb_Library_add_exception_objects,WinSpell,\
	lingucomponent/source/spellcheck/winspell/winspellimp \
))

# vim: set noet sw=4 ts=4:
