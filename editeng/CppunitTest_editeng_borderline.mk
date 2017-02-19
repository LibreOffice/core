# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

$(eval $(call gb_CppunitTest_CppunitTest,editeng_borderline))

$(eval $(call gb_CppunitTest_add_exception_objects,editeng_borderline, \
    editeng/qa/items/borderline_test \
))

$(eval $(call gb_CppunitTest_use_libraries,editeng_borderline, \
    xo \
    basegfx \
    editeng \
    lng \
    svt \
    tk \
    vcl \
    svl \
    sot \
    utl \
    tl \
    comphelper \
    ucbhelper \
    cppuhelper \
    cppu \
    sal \
    salhelper \
    i18nlangtag \
    i18nutil \
))

$(eval $(call gb_CppunitTest_use_externals,editeng_borderline,\
	boost_headers \
    icuuc \
))

$(eval $(call gb_CppunitTest_use_sdk_api,editeng_borderline))

# vim: set noet sw=4 ts=4:
