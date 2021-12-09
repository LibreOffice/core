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

$(eval $(call gb_Library_Library,msword))

$(eval $(call gb_Library_set_componentfile,msword,sw/util/msword,services))

$(eval $(call gb_Library_use_custom_headers,msword,\
	officecfg/registry \
	oox/generated \
))

$(eval $(call gb_Library_set_precompiled_header,msword,sw/inc/pch/precompiled_msword))

$(eval $(call gb_Library_set_include,msword,\
    -I$(SRCDIR)/sw/source/filter/inc \
    -I$(SRCDIR)/sw/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_Library_use_api,msword,\
	udkapi \
	offapi \
	oovbaapi \
))

$(eval $(call gb_Library_use_libraries,msword,\
    basegfx \
    comphelper \
    cppu \
    cppuhelper \
    editeng \
    i18nlangtag \
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
))

$(eval $(call gb_Library_use_externals,msword,\
	boost_headers \
	icui18n \
	icuuc \
	icu_headers \
	libxml2 \
))

$(eval $(call gb_Library_add_exception_objects,msword,\
    sw/source/filter/docx/swdocxreader \
    sw/source/filter/rtf/swparrtf \
    sw/source/filter/ww8/docxattributeoutput \
    sw/source/filter/ww8/docxexport \
    sw/source/filter/ww8/docxexportfilter \
    sw/source/filter/ww8/docxsdrexport \
    sw/source/filter/ww8/docxtablestyleexport \
    sw/source/filter/ww8/rtfattributeoutput \
    sw/source/filter/ww8/rtfexport \
    sw/source/filter/ww8/rtfexportfilter \
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
