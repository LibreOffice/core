# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# Copyright the LibreOffice contributors.
#
# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,chart2_correlation_coefficient))

$(eval $(call gb_CppunitTest_add_exception_objects,chart2_correlation_coefficient, \
    chart2/qa/unit/correlation_coefficient_test \
))

$(eval $(call gb_CppunitTest_use_libraries,chart2_correlation_coefficient, \
    sal \
))

$(eval $(call gb_CppunitTest_set_include,chart2_correlation_coefficient,\
    -I$(SRCDIR)/chart2/inc \
    -I$(SRCDIR)/chart2/source/inc \
    $$(INCLUDE) \
))

# vim: set noet sw=4 ts=4:
