# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,testuno))

$(eval $(call gb_Library_add_exception_objects,testuno, \
    unotest/source/testuno/testuno \
    unotest/source/testuno/serviceconstructors \
    unotest/source/testuno/singleton \
))

$(eval $(call gb_Library_set_componentfile,testuno,unotest/source/testuno/testuno,services))

$(eval $(call gb_Library_use_libraries,testuno, \
    cppu \
    cppuhelper \
    sal \
    salhelper \
    tl \
    vcl \
    $(if $(filter -fsanitize=vptr,$(gb_CXX)),comphelper) \
))

$(eval $(call gb_Library_use_sdk_api,testuno))

# vim: set noet sw=4 ts=4:
