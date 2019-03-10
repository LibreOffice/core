# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#*************************************************************************
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#*************************************************************************

$(eval $(call gb_CppunitTest_CppunitTest,sc_datapilotfieldgroupobj))

$(eval $(call gb_CppunitTest_use_external,sc_datapilotfieldgroupobj,boost_headers))

$(eval $(call gb_CppunitTest_add_exception_objects,sc_datapilotfieldgroupobj, \
	sc/qa/extras/scdatapilotfieldgroupobj \
))

$(eval $(call gb_CppunitTest_use_libraries,sc_datapilotfieldgroupobj, \
	comphelper \
	cppu \
	sal \
	subsequenttest \
	test \
	unotest \
))

$(eval $(call gb_CppunitTest_set_include,sc_datapilotfieldgroupobj,\
	$$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,sc_datapilotfieldgroupobj,\
	offapi \
	udkapi \
))

$(eval $(call gb_CppunitTest_use_ure,sc_datapilotfieldgroupobj))
$(eval $(call gb_CppunitTest_use_vcl,sc_datapilotfieldgroupobj))

$(eval $(call gb_CppunitTest_use_rdb,sc_datapilotfieldgroupobj,services))

$(eval $(call gb_CppunitTest_use_components,sc_datapilotfieldgroupobj))

$(eval $(call gb_CppunitTest_use_configuration,sc_datapilotfieldgroupobj))

# vim: set noet sw=4 ts=4:
