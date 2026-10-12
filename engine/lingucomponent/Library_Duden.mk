# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,Duden))

$(eval $(call gb_Library_set_componentfile,Duden,lingucomponent/source/spellcheck/duden/Duden,services))

$(eval $(call gb_Library_set_include,Duden,\
	$$(INCLUDE) \
	-I$(SRCDIR)/lingucomponent/source/lingutil \
))

$(eval $(call gb_Library_use_sdk_api,Duden))

$(eval $(call gb_Library_use_libraries,Duden,\
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

$(eval $(call gb_Library_use_static_libraries,Duden,\
	ulinguremote \
))

$(eval $(call gb_Library_use_externals,Duden,\
	boost_headers \
	curl \
))

$(eval $(call gb_Library_use_custom_headers,Duden,\
	officecfg/registry \
))

$(eval $(call gb_Library_add_exception_objects,Duden,\
	lingucomponent/source/spellcheck/duden/dudenimp \
))

# vim: set noet sw=4 ts=4:
