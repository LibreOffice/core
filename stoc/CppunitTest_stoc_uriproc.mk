# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CppunitTest_CppunitTest,stoc_uriproc))

$(eval $(call gb_CppunitTest_add_exception_objects,stoc_uriproc, \
    stoc/test/uriproc/test_uriproc \
))

$(eval $(call gb_CppunitTest_use_libraries,stoc_uriproc, \
    cppu \
    cppuhelper \
    sal \
))

$(eval $(call gb_CppunitTest_use_udk_api,stoc_uriproc))

$(eval $(call gb_CppunitTest_use_ure,stoc_uriproc))

# In a full build this is already covered indirectly by gb_CppunitTest_use_ure, but a module-only
# build needs it (and instead using gb_CppunitTest_use_component would cause a "duplicate
# implementation" DeploymentException):
$(call gb_CppunitTest_get_target,stoc_uriproc): \
    $(call gb_ComponentTarget_get_target,stoc/util/stocservices)

# vim: set noet sw=4 ts=4:
