# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,testtools_bridgetest_s))

$(eval $(call gb_StaticLibrary_use_external,testtools_bridgetest_s,boost_headers))

$(eval $(call gb_StaticLibrary_use_internal_api,testtools_bridgetest_s,\
    bridgetest \
))

$(eval $(call gb_StaticLibrary_use_api,testtools_bridgetest_s,\
    udkapi \
))

$(eval $(call gb_StaticLibrary_add_exception_objects,testtools_bridgetest_s,\
    testtools/source/bridgetest/currentcontextchecker \
    testtools/source/bridgetest/multi \
))

# vim: set noet sw=4 ts=4:
