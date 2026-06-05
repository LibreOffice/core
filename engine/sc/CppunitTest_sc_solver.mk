# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,sc_solver))

$(eval $(call gb_CppunitTest_add_exception_objects,sc_solver,\
	sc/qa/unit/solver \
))

$(eval $(call gb_CppunitTest_add_defs,sc_solver,\
	$(if $(ENABLE_COINMP), -DENABLE_COINMP) \
))

$(eval $(call gb_CppunitTest_use_libraries,sc_solver,\
	cppu \
	sal \
	test \
	unotest \
))

$(eval $(call gb_CppunitTest_set_include,sc_solver,\
	-I$(SRCDIR)/sc/inc \
	$$(INCLUDE) \
))


$(eval $(call gb_CppunitTest_use_sdk_api,sc_solver))

$(eval $(call gb_CppunitTest_use_ure,sc_solver))
$(eval $(call gb_CppunitTest_use_vcl,sc_solver))

$(eval $(call gb_CppunitTest_use_rdb,sc_solver,services))

$(eval $(call gb_CppunitTest_use_configuration,sc_solver))

# vim: set noet sw=4 ts=4:
