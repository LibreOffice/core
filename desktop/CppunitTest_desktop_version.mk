# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,desktop_version))

$(eval $(call gb_CppunitTest_add_exception_objects,desktop_version, \
    desktop/qa/deployment_misc/test_dp_version \
))

$(eval $(call gb_CppunitTest_use_libraries,desktop_version, \
    deploymentmisc \
    sal \
))

$(eval $(call gb_CppunitTest_use_udk_api,desktop_version))

# vim: set noet sw=4 ts=4:
