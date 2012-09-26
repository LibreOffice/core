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

$(eval $(call gb_Library_Library,msword))

$(eval $(call gb_Library_set_componentfile,msword,sw/util/msword))

$(eval $(call gb_Library_use_package,msword,writerfilter_sprmids))

$(eval $(call gb_Library_set_include,msword,\
    -I$(SRCDIR)/sw/source/core/inc \
    -I$(SRCDIR)/sw/source/ui/inc \
    -I$(SRCDIR)/sw/source/filter/inc \
    -I$(SRCDIR)/sw/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_Library_use_sdk_api,msword))

$(eval $(call gb_Library_use_libraries,msword,\
    basegfx \
    comphelper \
    cppu \
    cppuhelper \
    editeng \
    i18nisolang1 \
    i18nutil \
    msfilter \
    sal \
    sax \
    sb \
    oox \
    sfx \
    sot \
    svl \
    svt \
    svx \
    svxcore \
    sw \
    tk \
    tl \
    ucbhelper \
    utl \
    vcl \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_standard_system_libs,msword))

$(eval $(call gb_Library_use_externals,msword,\
	icuuc \
	libxml2 \
))

$(eval $(call gb_Library_add_exception_objects,msword,\
    sw/source/filter/rtf/rtffld \
    sw/source/filter/rtf/rtffly \
    sw/source/filter/rtf/rtfnum \
    sw/source/filter/rtf/rtftbl \
    sw/source/filter/rtf/swparrtf \
    sw/source/filter/ww8/docxattributeoutput \
    sw/source/filter/ww8/docxexport \
    sw/source/filter/ww8/docxexportfilter \
    sw/source/filter/ww8/rtfattributeoutput \
    sw/source/filter/ww8/rtfexport \
    sw/source/filter/ww8/rtfexportfilter \
    sw/source/filter/ww8/rtfimportfilter \
    sw/source/filter/ww8/rtfsdrexport \
    sw/source/filter/ww8/rtfstringbuffer \
    sw/source/filter/ww8/WW8FFData \
    sw/source/filter/ww8/WW8FibData \
    sw/source/filter/ww8/WW8Sttbf \
    sw/source/filter/ww8/WW8TableInfo \
    sw/source/filter/ww8/fields \
    sw/source/filter/ww8/styles \
    sw/source/filter/ww8/writerhelper \
    sw/source/filter/ww8/writerwordglue \
    sw/source/filter/ww8/wrtw8esh \
    sw/source/filter/ww8/wrtw8nds \
    sw/source/filter/ww8/wrtw8num \
    sw/source/filter/ww8/wrtw8sty \
    sw/source/filter/ww8/wrtww8 \
    sw/source/filter/ww8/wrtww8gr \
    sw/source/filter/ww8/ww8atr \
    sw/source/filter/ww8/ww8glsy \
    sw/source/filter/ww8/ww8graf \
    sw/source/filter/ww8/ww8graf2 \
    sw/source/filter/ww8/ww8par \
    sw/source/filter/ww8/ww8par2 \
    sw/source/filter/ww8/ww8par3 \
    sw/source/filter/ww8/ww8par4 \
    sw/source/filter/ww8/ww8par5 \
    sw/source/filter/ww8/ww8par6 \
    sw/source/filter/ww8/ww8scan \
    sw/source/filter/ww8/ww8toolbar \
))

# vim: set noet sw=4 ts=4:
