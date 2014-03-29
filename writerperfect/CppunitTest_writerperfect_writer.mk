# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,writerperfect_writer))

$(eval $(call gb_CppunitTest_use_externals,writerperfect_writer,\
	boost_headers \
))

$(eval $(call gb_CppunitTest_use_api,writerperfect_writer,\
	offapi \
	udkapi \
))

$(eval $(call gb_CppunitTest_use_libraries,writerperfect_writer,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	test \
	ucbhelper \
	unotest \
	$(gb_UWINAPI) \
))

$(eval $(call gb_CppunitTest_use_ure,writerperfect_writer))

$(eval $(call gb_CppunitTest_use_rdb,writerperfect_writer,services))

$(eval $(call gb_CppunitTest_use_configuration,writerperfect_writer))

$(eval $(call gb_CppunitTest_add_exception_objects,writerperfect_writer,\
	writerperfect/qa/unit/wpftwriter \
))

# vim: set noet sw=4 ts=4:
