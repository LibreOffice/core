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

$(eval $(call gb_Library_Library,textconversiondlgs))

$(eval $(call gb_Library_set_componentfile,textconversiondlgs,svx/util/textconversiondlgs))

$(eval $(call gb_Library_set_include,textconversiondlgs,\
    -I$(SRCDIR)/svx/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_Library_use_sdk_api,textconversiondlgs))

$(eval $(call gb_Library_use_libraries,textconversiondlgs,\
    comphelper \
    cppuhelper \
    cppu \
    sal \
    svl \
    svt \
    tk \
    tl \
    utl \
    vcl \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_standard_system_libs,textconversiondlgs))

$(eval $(call gb_Library_add_exception_objects,textconversiondlgs,\
    svx/source/unodialogs/textconversiondlgs/services \
    svx/source/unodialogs/textconversiondlgs/chinese_dictionarydialog \
    svx/source/unodialogs/textconversiondlgs/chinese_translationdialog \
    svx/source/unodialogs/textconversiondlgs/chinese_translation_unodialog \
    svx/source/unodialogs/textconversiondlgs/resid \
))

# vim: set noet sw=4 ts=4:
