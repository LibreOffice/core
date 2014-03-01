# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,writerfilter_misc))

$(eval $(call gb_CppunitTest_use_api,writerfilter_misc,\
	offapi \
	udkapi \
))

$(eval $(call gb_CppunitTest_use_external,writerfilter_misc,boost_headers))

$(eval $(call gb_CppunitTest_use_libraries,writerfilter_misc, \
	writerfilter \
	cppu \
	sal \
	$(gb_UWINAPI) \
))

$(eval $(call gb_CppunitTest_set_include,writerfilter_misc, \
	$$(INCLUDE) \
	-I$(SRCDIR)/writerfilter/inc \
))

$(eval $(call gb_CppunitTest_add_exception_objects,writerfilter_misc, \
	writerfilter/qa/cppunittests/misc/misc \
))


# vim: set noet sw=4 ts=4:
