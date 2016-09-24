# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,libreofficekit_checkapi))

$(eval $(call gb_CppunitTest_add_cxxflags,libreofficekit_checkapi, \
    $(gb_CXX03FLAGS) \
))

$(eval $(call gb_CppunitTest_add_exception_objects,libreofficekit_checkapi, \
    libreofficekit/qa/unit/checkapi \
))

$(eval $(call gb_CppunitTest_set_external_code,libreofficekit_checkapi))

ifeq ($(OS),LINUX)
$(eval $(call gb_CppunitTest_add_libs,libreofficekit_checkapi, \
    -ldl \
))
endif

# vim: set noet sw=4 ts=4:
