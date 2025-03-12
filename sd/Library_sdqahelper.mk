# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,sdqahelper))

$(eval $(call gb_Library_set_include,sdqahelper,\
    -I$(SRCDIR)/sd/inc \
    -I$(SRCDIR)/sd/source/ui/inc \
    -I$(SRCDIR)/sd/qa/inc \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_use_custom_headers,sdqahelper,\
	officecfg/registry \
))

$(eval $(call gb_Library_use_externals,sdqahelper, \
	boost_headers \
	cppunit \
	libxml2 \
))

$(eval $(call gb_Library_add_defs,sdqahelper,\
	-DSDQAHELPER_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_use_api,sdqahelper,\
	udkapi \
	offapi \
	oovbaapi \
))

$(eval $(call gb_Library_use_libraries,sdqahelper,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	subsequenttest \
	svxcore \
	sd \
	sfx \
	tl \
	utl \
	vcl \
	test \
	unotest \
))

$(eval $(call gb_Library_add_exception_objects,sdqahelper,\
	sd/qa/unit/sdtiledrenderingtest \
))

# vim: set noet sw=4 ts=4:
