# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,tubes_test))

$(eval $(call gb_CppunitTest_add_exception_objects,tubes_test, \
	tubes/qa/test_manager \
))

$(eval $(call gb_CppunitTest_use_libraries,tubes_test, \
	sal \
	tubes \
	utl \
	$(gb_UWINAPI) \
))

$(eval $(call gb_CppunitTest_use_externals,tubes_test,\
	telepathy \
))

$(eval $(call gb_CppunitTest_use_udk_api,tubes_test))

# vim: set noet sw=4 ts=4:
