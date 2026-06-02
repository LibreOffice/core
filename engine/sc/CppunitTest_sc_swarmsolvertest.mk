# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,sc_swarmsolvertest))

$(eval $(call gb_CppunitTest_add_exception_objects,sc_swarmsolvertest,\
	sc/qa/unit/SwarmSolverTest \
))

$(eval $(call gb_CppunitTest_use_libraries,sc_swarmsolvertest,\
	basegfx \
	comphelper \
	cppu \
	cppuhelper \
	drawinglayer \
	editeng \
	for \
	forui \
	i18nlangtag \
	msfilter \
	oox \
	sal \
	salhelper \
	sax \
	sb \
	sc \
	scqahelper \
	sfx \
	sot \
	subsequenttest \
	svl \
	svt \
	svx \
	svxcore \
	test \
	tk \
	tl \
	ucbhelper \
	unotest \
	utl \
    $(call gb_Helper_optional,SCRIPTING, \
        vbahelper) \
	vcl \
	xo \
	$(gb_UWINAPI) \
))

$(eval $(call gb_CppunitTest_set_include,sc_swarmsolvertest,\
	-I$(SRCDIR)/sc/inc \
	-I$(SRCDIR)/sc/source/solver \
	$$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_sdk_api,sc_swarmsolvertest))

$(eval $(call gb_CppunitTest_use_ure,sc_swarmsolvertest))
$(eval $(call gb_CppunitTest_use_vcl,sc_swarmsolvertest))

$(eval $(call gb_CppunitTest_use_rdb,sc_swarmsolvertest,services))

$(eval $(call gb_CppunitTest_use_configuration,sc_swarmsolvertest))

# vim: set noet sw=4 ts=4:
