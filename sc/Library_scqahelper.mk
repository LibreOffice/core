# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,scqahelper))

$(eval $(call gb_Library_set_include,scqahelper,\
	-I$(SRCDIR)/sc/inc \
	-I$(SRCDIR)/sc/source/ui/inc \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_use_externals,scqahelper, \
	boost_headers \
	mdds_headers \
	cppunit \
	libxml2 \
))

ifneq ($(SYSTEM_LIBORCUS),)
$(eval $(call gb_Library_use_externals,scqahelper,orcus))
else
$(eval $(call gb_Library_use_externals,scqahelper,orcus-parser))
endif

$(eval $(call gb_Library_add_defs,scqahelper,\
	-DSCQAHELPER_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_use_sdk_api,scqahelper))

$(eval $(call gb_Library_use_libraries,scqahelper,\
	comphelper \
	cppu \
	cppuhelper \
	editeng \
	for \
	sal \
	sc \
	sfx \
	sot \
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
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_exception_objects,scqahelper,\
	sc/qa/unit/helper/qahelper \
	sc/qa/unit/helper/xpath \
))

# vim: set noet sw=4 ts=4:
