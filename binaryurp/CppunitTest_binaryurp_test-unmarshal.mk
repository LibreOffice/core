# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,binaryurp_test-unmarshal))

$(eval $(call gb_CppunitTest_use_libraries,binaryurp_test-unmarshal,\
	cppu \
	cppuhelper \
	sal \
	salhelper \
))

$(eval $(call gb_CppunitTest_use_library_objects,binaryurp_test-unmarshal,\
	binaryurp \
))

$(eval $(call gb_CppunitTest_use_api,binaryurp_test-unmarshal,\
	udkapi \
))

$(eval $(call gb_CppunitTest_add_exception_objects,binaryurp_test-unmarshal,\
	binaryurp/qa/test-unmarshal \
))

# vim: set noet sw=4 ts=4:
