# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,sccomp_lpsolver))

$(eval $(call gb_CppunitTest_add_exception_objects,sccomp_lpsolver,\
	sccomp/qa/unit/lpsolver \
))

$(eval $(call gb_CppunitTest_use_externals,sccomp_lpsolver,\
	boost_headers \
))

$(eval $(call gb_CppunitTest_add_defs,sccomp_lpsolver,\
	$(if $(ENABLE_COINMP), -DENABLE_COINMP) \
	$(if $(ENABLE_LPSOLVE), -DENABLE_LPSOLVE) \
))

$(eval $(call gb_CppunitTest_use_libraries,sccomp_lpsolver,\
	cppu \
	sal \
	test \
	$(gb_UWINAPI) \
))

$(eval $(call gb_CppunitTest_set_include,sccomp_lpsolver,\
	-I$(SRCDIR)/sc/inc \
	$$(INCLUDE) \
))


$(eval $(call gb_CppunitTest_use_api,sccomp_lpsolver,\
	offapi \
	udkapi \
))

$(eval $(call gb_CppunitTest_use_ure,sccomp_lpsolver))

$(eval $(call gb_CppunitTest_use_rdb,sccomp_lpsolver,services))

$(eval $(call gb_CppunitTest_use_configuration,sccomp_lpsolver))

# vim: set noet sw=4 ts=4:
