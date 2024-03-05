# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,sccomp_solver))

$(eval $(call gb_CppunitTest_add_exception_objects,sccomp_solver,\
	sccomp/qa/unit/solver \
))

$(eval $(call gb_CppunitTest_add_defs,sccomp_solver,\
	$(if $(ENABLE_COINMP), -DENABLE_COINMP) \
	$(if $(ENABLE_LPSOLVE), -DENABLE_LPSOLVE) \
))

$(eval $(call gb_CppunitTest_use_libraries,sccomp_solver,\
	cppu \
	sal \
	test \
	unotest \
))

$(eval $(call gb_CppunitTest_set_include,sccomp_solver,\
	-I$(SRCDIR)/sc/inc \
	$$(INCLUDE) \
))


$(eval $(call gb_CppunitTest_use_sdk_api,sccomp_solver))

$(eval $(call gb_CppunitTest_use_ure,sccomp_solver))
$(eval $(call gb_CppunitTest_use_vcl,sccomp_solver))

$(eval $(call gb_CppunitTest_use_rdb,sccomp_solver,services))

$(eval $(call gb_CppunitTest_use_configuration,sccomp_solver))

# vim: set noet sw=4 ts=4:
