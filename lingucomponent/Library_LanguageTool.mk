# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,LanguageTool))

$(eval $(call gb_Library_set_componentfile,LanguageTool,lingucomponent/source/spellcheck/languagetool/LanguageTool,services))

$(eval $(call gb_Library_set_include,LanguageTool,\
	$$(INCLUDE) \
	-I$(SRCDIR)/lingucomponent/source/lingutil \
))

$(eval $(call gb_Library_use_sdk_api,LanguageTool))

$(eval $(call gb_Library_use_libraries,LanguageTool,\
	comphelper \
	cppu \
	cppuhelper \
	fwk \
	i18nlangtag \
	svt \
	lng \
	sal \
	tl \
	utl \
))

$(eval $(call gb_Library_use_static_libraries,LanguageTool,\
	ulingu \
))

$(eval $(call gb_Library_use_externals,LanguageTool,\
	boost_headers \
	icuuc \
	curl \
))

$(eval $(call gb_Library_use_custom_headers,LanguageTool,\
	officecfg/registry \
))

$(eval $(call gb_Library_add_exception_objects,LanguageTool,\
	lingucomponent/source/spellcheck/languagetool/languagetoolimp \
))

# vim: set noet sw=4 ts=4:
