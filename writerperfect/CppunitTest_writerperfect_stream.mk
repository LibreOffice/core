# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,writerperfect_stream))

$(eval $(call gb_CppunitTest_set_include,writerperfect_stream,\
	-I$(SRCDIR)/writerperfect/source/common \
	$$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,writerperfect_stream,\
	offapi \
	udkapi \
))

$(eval $(call gb_CppunitTest_use_ure,writerperfect_stream))

$(eval $(call gb_CppunitTest_use_externals,writerperfect_stream,\
	boost_headers \
	wpd \
))

$(eval $(call gb_CppunitTest_use_libraries,writerperfect_stream,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	sot \
	test \
	tl \
	utl \
))

$(eval $(call gb_CppunitTest_use_static_libraries,writerperfect_stream,\
	writerperfect \
))

$(eval $(call gb_CppunitTest_use_configuration,writerperfect_stream))

$(eval $(call gb_CppunitTest_use_components,writerperfect_stream,\
	configmgr/source/configmgr \
	ucb/source/core/ucb1 \
    ucb/source/ucp/file/ucpfile1 \
))

$(eval $(call gb_CppunitTest_add_exception_objects,writerperfect_stream,\
	writerperfect/qa/unit/WPXSvStreamTest \
))

# vim: set noet sw=4 ts=4:
