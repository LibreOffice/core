# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,testtools_constructors))

$(eval $(call gb_Library_set_componentfile,testtools_constructors,testtools/source/bridgetest/constructors))

$(eval $(call gb_Executable_set_include,testtools_constructors,\
    -I$(SRCDIR)/testtools/source/bridgetest \
    $$(INCLUDE) \
))

$(eval $(call gb_Library_use_external,testtools_constructors,boost_headers))

$(eval $(call gb_Library_use_internal_api,testtools_constructors,\
    bridgetest \
))

$(eval $(call gb_Library_use_api,testtools_constructors,\
    udkapi \
))

$(eval $(call gb_Library_use_libraries,testtools_constructors,\
    cppu \
    cppuhelper \
    sal \
))

$(eval $(call gb_Library_add_exception_objects,testtools_constructors,\
    testtools/source/bridgetest/constructors \
))

# vim:set noet sw=4 ts=4:
