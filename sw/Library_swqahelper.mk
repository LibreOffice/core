# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,swqahelper))

$(eval $(call gb_Library_set_include,swqahelper,\
	-I$(SRCDIR)/sw/inc \
	-I$(SRCDIR)/sw/source/core/inc \
	-I$(SRCDIR)/sw/source/uibase/inc \
	-I$(SRCDIR)/sw/qa/inc \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_use_custom_headers,swqahelper,\
	officecfg/registry \
))

$(eval $(call gb_Library_use_externals,swqahelper, \
	boost_headers \
	cppunit \
	libxml2 \
))

$(eval $(call gb_Library_use_custom_headers,swqahelper,\
	officecfg/registry \
))

$(eval $(call gb_Library_add_defs,swqahelper,\
	-DSWQAHELPER_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_use_api,swqahelper,\
	udkapi \
	offapi \
	oovbaapi \
))

$(eval $(call gb_Library_use_libraries,swqahelper,\
	comphelper \
	cppu \
	cppuhelper \
	editeng \
	for \
	sal \
	sw \
	sfx \
	sot \
	subsequenttest \
	svl \
	svt \
	svx \
	svxcore \
	test \
	tl \
	unotest \
	utl \
	ucbhelper \
	unotest \
	vcl \
))

$(eval $(call gb_Library_add_exception_objects,swqahelper,\
	sw/qa/unit/swmodeltestbase \
	sw/qa/unit/swtiledrenderingtest \
	sw/qa/unit/swtestviewcallback \
))

# vim: set noet sw=4 ts=4:
