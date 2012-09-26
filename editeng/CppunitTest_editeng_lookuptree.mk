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

$(eval $(call gb_CppunitTest_CppunitTest,editeng_lookuptree))

$(eval $(call gb_CppunitTest_add_exception_objects,editeng_lookuptree, \
    editeng/qa/lookuptree/lookuptree_test \
))

$(eval $(call gb_CppunitTest_use_libraries,editeng_lookuptree, \
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
    i18nisolang1 \
    i18nutil \
	$(gb_UWINAPI) \
))

$(eval $(call gb_CppunitTest_add_standard_system_libs,editeng_lookuptree))

$(eval $(call gb_CppunitTest_use_externals,editeng_lookuptree,\
    icuuc \
))

$(eval $(call gb_CppunitTest_set_include,editeng_lookuptree,\
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,editeng_lookuptree,\
    offapi \
    udkapi \
))

# vim: set noet sw=4 ts=4:
