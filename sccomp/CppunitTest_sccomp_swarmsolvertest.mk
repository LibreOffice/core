# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,swarm_solver_test))

$(eval $(call gb_CppunitTest_add_exception_objects,swarm_solver_test,\
	sccomp/qa/unit/SwarmSolverTest \
))

$(eval $(call gb_CppunitTest_use_externals,swarm_solver_test,\
	boost_headers \
))

$(eval $(call gb_CppunitTest_use_libraries,swarm_solver_test,\
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
	vbahelper \
	vcl \
	xo \
	$(gb_UWINAPI) \
))

$(eval $(call gb_CppunitTest_set_include,swarm_solver_test,\
	-I$(SRCDIR)/sc/inc \
	$$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_sdk_api,swarm_solver_test))

$(eval $(call gb_CppunitTest_use_ure,swarm_solver_test))
$(eval $(call gb_CppunitTest_use_vcl,swarm_solver_test))

$(eval $(call gb_CppunitTest_use_rdb,swarm_solver_test,services))

$(eval $(call gb_CppunitTest_use_configuration,swarm_solver_test))

# vim: set noet sw=4 ts=4:
