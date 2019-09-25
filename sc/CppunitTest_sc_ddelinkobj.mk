# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#*************************************************************************
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#*************************************************************************

$(eval $(call gb_CppunitTest_CppunitTest,sc_ddelinkobj))

$(eval $(call gb_CppunitTest_use_external,sc_ddelinkobj,boost_headers))

$(eval $(call gb_CppunitTest_use_common_precompiled_header,sc_ddelinkobj))

$(eval $(call gb_CppunitTest_add_exception_objects,sc_ddelinkobj, \
	sc/qa/extras/scddelinkobj \
))

$(eval $(call gb_CppunitTest_use_libraries,sc_ddelinkobj, \
	cppu \
	sal \
	subsequenttest \
	test \
	unotest \
	utl \
	vcl \
))

$(eval $(call gb_CppunitTest_set_include,sc_ddelinkobj,\
	$$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_sdk_api,sc_ddelinkobj))

$(eval $(call gb_CppunitTest_use_ure,sc_ddelinkobj))
$(eval $(call gb_CppunitTest_use_vcl,sc_ddelinkobj))

$(eval $(call gb_CppunitTest_use_components,sc_ddelinkobj,\
    $(sc_unoapi_common_components) \
))

$(eval $(call gb_CppunitTest_use_configuration,sc_ddelinkobj))

# vim: set noet sw=4 ts=4:
