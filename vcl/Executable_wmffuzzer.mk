# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,wmffuzzer))

$(eval $(call gb_Executable_use_api,wmffuzzer,\
    offapi \
    udkapi \
))

$(eval $(call gb_Executable_use_externals,wmffuzzer,\
	boost_headers \
	curl \
	harfbuzz \
	graphite \
	cairo \
	fontconfig \
	freetype \
	icui18n \
	icuuc \
	icudata \
	lcms2 \
	librdf \
	libxslt \
	libxml2 \
	jpeg \
	clew \
	openssl \
	expat \
	mythes \
	hyphen \
	hunspell \
	zlib \
))

$(eval $(call gb_Executable_set_include,wmffuzzer,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
))

$(eval $(call gb_Executable_use_libraries,wmffuzzer,\
    basctl \
    merged \
    cui \
    chartcontroller \
    chartcore \
    sm \
    gie \
    oox \
    reflection \
    odfflatxml \
    invocadapt \
    bootstrap \
    introspection \
    stocservices \
    lnth \
    hyphen \
    i18nsearch \
    embobj \
    evtatt \
    unordf \
    ucphier1 \
    ucptdoc1 \
    srtrs1 \
    storagefd \
    mtfrenderer \
    canvasfactory \
    vclcanvas \
    xof \
    xmlfa \
    xmlfd \
    cppu \
    cppuhelper \
    comphelper \
    i18nlangtag \
    xmlreader \
    unoidl \
    reg \
    store \
    expwrap \
    gcc3_uno \
    salhelper \
    sal \
))

$(eval $(call gb_Executable_use_static_libraries,wmffuzzer,\
    findsofficepath \
    ulingu \
))

$(eval $(call gb_Executable_add_exception_objects,wmffuzzer,\
	vcl/workben/wmffuzzer \
	vcl/workben/localestub/localestub \
	vcl/workben/localestub/localedata_en_AU \
	vcl/workben/localestub/localedata_en_BW \
	vcl/workben/localestub/localedata_en_BZ \
	vcl/workben/localestub/localedata_en_CA \
	vcl/workben/localestub/localedata_en_GB \
	vcl/workben/localestub/localedata_en_GH \
	vcl/workben/localestub/localedata_en_GM \
	vcl/workben/localestub/localedata_en_IE \
	vcl/workben/localestub/localedata_en_IN \
	vcl/workben/localestub/localedata_en_JM \
	vcl/workben/localestub/localedata_en_MW \
	vcl/workben/localestub/localedata_en_NA \
	vcl/workben/localestub/localedata_en_NZ \
	vcl/workben/localestub/localedata_en_PH \
	vcl/workben/localestub/localedata_en_TT \
	vcl/workben/localestub/localedata_en_US \
	vcl/workben/localestub/localedata_en_ZA \
	vcl/workben/localestub/localedata_en_ZW \
))

$(eval $(call gb_Executable_add_generated_exception_objects,wmffuzzer,\
    CustomTarget/vcl/workben/native-code \
))

$(eval $(call gb_Executable_add_libs,wmffuzzer,\
	-lFuzzingEngine \
))

# vim: set noet sw=4 ts=4:
