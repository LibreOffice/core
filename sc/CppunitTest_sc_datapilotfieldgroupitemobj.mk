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

$(eval $(call gb_CppunitTest_CppunitTest,sc_datapilotfieldgroupitemobj))

$(eval $(call gb_CppunitTest_use_external,sc_datapilotfieldgroupitemobj,boost_headers))

$(eval $(call gb_CppunitTest_add_exception_objects,sc_datapilotfieldgroupitemobj, \
	sc/qa/extras/scdatapilotfieldgroupitemobj \
))

$(eval $(call gb_CppunitTest_use_libraries,sc_datapilotfieldgroupitemobj, \
	comphelper \
	cppu \
	sal \
	subsequenttest \
	test \
	unotest \
))

$(eval $(call gb_CppunitTest_set_include,sc_datapilotfieldgroupitemobj,\
	$$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,sc_datapilotfieldgroupitemobj,\
	offapi \
	udkapi \
))

$(eval $(call gb_CppunitTest_use_ure,sc_datapilotfieldgroupitemobj))
$(eval $(call gb_CppunitTest_use_vcl,sc_datapilotfieldgroupitemobj))

$(eval $(call gb_CppunitTest_use_rdb,sc_datapilotfieldgroupitemobj,services))

$(eval $(call gb_CppunitTest_use_components,sc_datapilotfieldgroupitemobj))

$(eval $(call gb_CppunitTest_use_configuration,sc_datapilotfieldgroupitemobj))

# vim: set noet sw=4 ts=4:
