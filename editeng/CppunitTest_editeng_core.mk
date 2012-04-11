# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2011 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

$(eval $(call gb_CppunitTest_CppunitTest,editeng_core))

$(eval $(call gb_CppunitTest_add_exception_objects,editeng_core, \
    editeng/qa/unit/core-test \
))

$(eval $(call gb_CppunitTest_use_libraries,editeng_core, \
    basegfx \
    comphelper \
    cppu \
    cppuhelper \
    editeng \
    i18nisolang1 \
    i18nutil \
    lng \
    sal \
    salhelper \
    sot \
    svl \
    svt \
    test \
    tk \
    tl \
    ucbhelper \
    utl \
    vcl \
    xo \
    $(gb_STDLIBS) \
))

$(eval $(call gb_CppunitTest_use_externals,editeng_core,\
    icuuc \
))

$(eval $(call gb_CppunitTest_set_include,editeng_core,\
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,editeng_core,\
    offapi \
    udkapi \
))

$(eval $(call gb_CppunitTest_use_components,editeng_core,\
    configmgr/source/configmgr \
    framework/util/fwk \
    i18npool/util/i18npool \
    sfx2/util/sfx \
    ucb/source/core/ucb1 \
    ucb/source/ucp/file/ucpfile1 \
    unoxml/source/service/unoxml \
))

$(eval $(call gb_CppunitTest_use_configuration,editeng_core))

# vim: set noet sw=4 ts=4:
