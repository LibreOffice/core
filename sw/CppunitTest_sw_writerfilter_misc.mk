# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,sw_writerfilter_misc))

$(eval $(call gb_CppunitTest_use_sdk_api,sw_writerfilter_misc))

$(eval $(call gb_CppunitTest_use_custom_headers,sw_writerfilter_misc,\
	sw/source/writerfilter \
))

$(eval $(call gb_CppunitTest_set_include,sw_writerfilter_misc,\
    $$(INCLUDE) \
    -I$(SRCDIR)/sw/source/writerfilter/inc \
    -I$(SRCDIR)/sw/source/writerfilter/ \
    -I$(SRCDIR)/sw/source/writerfilter/dmapper \
))

$(eval $(call gb_CppunitTest_use_external,sw_writerfilter_misc,boost_headers))

$(eval $(call gb_CppunitTest_use_libraries,sw_writerfilter_misc, \
	sw_writerfilter \
	cppu \
	sal \
	salhelper \
))

$(eval $(call gb_CppunitTest_add_exception_objects,sw_writerfilter_misc, \
	sw/qa/writerfilter/misc/misc \
))


# vim: set noet sw=4 ts=4:
