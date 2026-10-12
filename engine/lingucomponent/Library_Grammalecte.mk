# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,Grammalecte))

$(eval $(call gb_Library_set_componentfile,Grammalecte,lingucomponent/source/spellcheck/grammalecte/Grammalecte,services))

$(eval $(call gb_Library_set_include,Grammalecte,\
	$$(INCLUDE) \
	-I$(SRCDIR)/lingucomponent/source/lingutil \
))

$(eval $(call gb_Library_use_sdk_api,Grammalecte))

$(eval $(call gb_Library_use_libraries,Grammalecte,\
	comphelper \
	cppu \
	cppuhelper \
	i18nlangtag \
	lng \
	sal \
	svt \
	tl \
	utl \
))

$(eval $(call gb_Library_use_static_libraries,Grammalecte,\
	ulinguremote \
))

$(eval $(call gb_Library_use_externals,Grammalecte,\
	boost_headers \
	curl \
))

$(eval $(call gb_Library_use_custom_headers,Grammalecte,\
	officecfg/registry \
))

$(eval $(call gb_Library_add_exception_objects,Grammalecte,\
	lingucomponent/source/spellcheck/grammalecte/grammalecteimp \
))

# vim: set noet sw=4 ts=4:
