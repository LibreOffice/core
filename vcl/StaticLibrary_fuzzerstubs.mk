# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

$(eval $(call gb_StaticLibrary_StaticLibrary,fuzzerstubs))

$(eval $(call gb_StaticLibrary_set_include,fuzzerstubs,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
))

$(eval $(call gb_StaticLibrary_use_api,fuzzerstubs,\
    offapi \
    udkapi \
))

$(eval $(call gb_StaticLibrary_add_exception_objects,fuzzerstubs,\
    vcl/workben/localestub/localestub \
    vcl/workben/localestub/localedata_en_AU \
    vcl/workben/localestub/localedata_en_BW \
    vcl/workben/localestub/localedata_en_BZ \
    vcl/workben/localestub/localedata_en_CA \
    vcl/workben/localestub/localedata_en_GB \
    vcl/workben/localestub/localedata_en_GH \
    vcl/workben/localestub/localedata_en_GM \
    vcl/workben/localestub/localedata_en_HK \
    vcl/workben/localestub/localedata_en_IE \
    vcl/workben/localestub/localedata_en_IN \
    vcl/workben/localestub/localedata_en_JM \
    vcl/workben/localestub/localedata_en_LK \
    vcl/workben/localestub/localedata_en_MW \
    vcl/workben/localestub/localedata_en_MY \
    vcl/workben/localestub/localedata_en_NA \
    vcl/workben/localestub/localedata_en_NG \
    vcl/workben/localestub/localedata_en_NZ \
    vcl/workben/localestub/localedata_en_PH \
    vcl/workben/localestub/localedata_en_TT \
    vcl/workben/localestub/localedata_en_US \
    vcl/workben/localestub/localedata_en_ZA \
    vcl/workben/localestub/localedata_en_ZM \
    vcl/workben/localestub/localedata_en_ZW \
))

# vim: set noet sw=4 ts=4:
