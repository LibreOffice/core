# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_InternalUnoApi_InternalUnoApi,bridgetest))

$(eval $(call gb_InternalUnoApi_use_api,bridgetest,\
    udkapi \
))

$(eval $(call gb_InternalUnoApi_set_include,bridgetest,\
    -I$(SRCDIR)/udkapi \
    $$(INCLUDE) \
))

$(eval $(call gb_InternalUnoApi_add_idlfiles,bridgetest,testtools/source/bridgetest/idl,\
    bridgetest \
))

$(eval $(call gb_InternalUnoApi_add_idlfiles,bridgetest,testtools/source/bridgetest/idl/com/sun/star/test/performance,\
    XPerformanceTest \
))

# vim:set noet sw=4 ts=4:
