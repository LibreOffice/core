# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,embobj))

$(eval $(call gb_Library_use_custom_headers,embobj,\
	officecfg/registry \
))

$(eval $(call gb_Library_set_componentfile,embobj,embeddedobj/util/embobj,services))

$(eval $(call gb_Library_set_include,embobj,\
	-I$(SRCDIR)/embeddedobj/source/inc \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_use_external,embobj,boost_headers))

$(eval $(call gb_Library_use_sdk_api,embobj))

$(eval $(call gb_Library_use_libraries,embobj,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	svt \
	utl \
	vcl \
	tl \
	i18nlangtag \
))

$(eval $(call gb_Library_add_exception_objects,embobj,\
	embeddedobj/source/commonembedding/embedobj \
	embeddedobj/source/commonembedding/inplaceobj \
	embeddedobj/source/commonembedding/miscobj \
	embeddedobj/source/commonembedding/persistence \
	embeddedobj/source/commonembedding/specialobject \
	embeddedobj/source/commonembedding/visobj \
	embeddedobj/source/commonembedding/xfactory \
	embeddedobj/source/general/docholder \
	embeddedobj/source/general/dummyobject \
	embeddedobj/source/general/intercept \
	embeddedobj/source/general/xcreator \
))

# vim: set noet sw=4 ts=4:
