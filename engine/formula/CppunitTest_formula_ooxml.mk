# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# Copyright the Collabora Office contributors.
#
# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,formula_ooxml))

$(eval $(call gb_CppunitTest_use_sdk_api,formula_ooxml))

$(eval $(call gb_CppunitTest_use_ure,formula_ooxml))

$(eval $(call gb_CppunitTest_set_include,formula_ooxml,\
	-I$(SRCDIR)/formula/inc \
	$$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_add_exception_objects,formula_ooxml,\
	formula/qa/unit/OOXMLRewriterTest \
))

$(eval $(call gb_CppunitTest_use_libraries,formula_ooxml,\
	comphelper \
	cppu \
	cppuhelper \
	for \
	i18nlangtag \
	sal \
	svl \
	svt \
	tl \
	utl \
	vcl \
))

$(eval $(call gb_CppunitTest_use_external,formula_ooxml,boost_headers))

# vim: set noet sw=4 ts=4:
