# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
 
$(eval $(call gb_CppunitTest_CppunitTest,scaddins_analysis))

$(eval $(call gb_CppunitTest_add_exception_objects,scaddins_analysis, \
	scaddins/qa/analysisTest \
))

$(eval $(call gb_Library_use_external,scaddins_analysis,boost_headers))

$(eval $(call gb_Library_use_common_precompiled_header,scaddins_analysis))

$(eval $(call gb_Library_use_internal_comprehensive_api,scaddins_analysis,\
	offapi \
	scaddins \
	udkapi \
))

$(eval $(call gb_CppunitTest_use_sdk_api,scaddins_analysis))
$(eval $(call gb_CppunitTest_use_ure,scaddins_analysis))
$(eval $(call gb_CppunitTest_use_vcl,scaddins_analysis))

# add a list of all needed libraries here
$(eval $(call gb_CppunitTest_add_linked_libs,scaddins_analysis, \
	comphelper \
	cppu \
	cppuhelper \
	com \
	sal \
	utl \
	i18nlangtag \
))

$(eval $(call gb_CppunitTest_set_include,scaddins_analysis,\
	$$(INCLUDE) \
	-I$(SRCDIR)/scaddins/source \
))

# vim: set noet sw=4 ts=4: