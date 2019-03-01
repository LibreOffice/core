# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,expwrap))

$(eval $(call gb_Library_set_componentfile,expwrap,sax/source/expatwrap/expwrap))

$(eval $(call gb_Library_set_include,expwrap,\
	-I$(SRCDIR)/sax/inc \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_add_defs,expwrap,\
	-DFASTSAX_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_use_sdk_api,expwrap))

$(eval $(call gb_Library_use_externals,expwrap,\
	boost_headers \
	libxml2 \
	expat \
	zlib \
))

$(eval $(call gb_Library_use_libraries,expwrap,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	salhelper \
	sax \
	tl \
))

$(eval $(call gb_Library_add_exception_objects,expwrap,\
	sax/source/expatwrap/sax_expat \
	sax/source/expatwrap/saxwriter \
	sax/source/expatwrap/xml2utf \
	sax/source/fastparser/fastparser \
	sax/source/fastparser/legacyfastparser \
))

# vim: set noet sw=4 ts=4:
