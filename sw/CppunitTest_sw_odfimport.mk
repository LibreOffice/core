# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#*************************************************************************
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#*************************************************************************

$(eval $(call gb_CppunitTest_CppunitTest,sw_odfimport))

$(eval $(call gb_CppunitTest_add_exception_objects,sw_odfimport, \
    sw/qa/extras/odfimport/odfimport \
))

$(eval $(call gb_CppunitTest_use_libraries,sw_odfimport, \
    comphelper \
    cppu \
    cppuhelper \
    sal \
	svt \
	sw \
    test \
    unotest \
    vcl \
    tl \
	$(gb_UWINAPI) \
))

$(eval $(call gb_CppunitTest_use_externals,sw_odfimport,\
	boost_headers \
    libxml2 \
))

$(eval $(call gb_CppunitTest_set_include,sw_odfimport,\
    -I$(SRCDIR)/sw/inc \
    -I$(SRCDIR)/sw/source/core/inc \
    -I$(SRCDIR)/sw/source/ui/inc \
	-I$(SRCDIR)/sw/qa/extras/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_CppunitTest_use_api,sw_odfimport,\
    offapi \
    udkapi \
))

$(eval $(call gb_CppunitTest_use_ure,sw_odfimport))

$(eval $(call gb_CppunitTest_use_components,sw_odfimport,\
	basic/util/sb \
    comphelper/util/comphelp \
    configmgr/source/configmgr \
	embeddedobj/util/embobj \
    fileaccess/source/fileacc \
    filter/source/config/cache/filterconfig1 \
    framework/util/fwk \
    i18npool/util/i18npool \
    linguistic/source/lng \
    package/util/package2 \
    package/source/xstor/xstor \
    sw/util/sw \
    sw/util/swd \
    sax/source/expatwrap/expwrap \
    sfx2/util/sfx \
    svl/source/fsstor/fsstorage \
    svtools/util/svt \
    toolkit/util/tk \
    ucb/source/core/ucb1 \
    ucb/source/ucp/file/ucpfile1 \
    unotools/util/utl \
    unoxml/source/service/unoxml \
    uui/util/uui \
    $(if $(filter-out MACOSX WNT,$(OS)), \
		vcl/vcl.unx \
    ) \
	$(if $(filter DESKTOP,$(BUILD_TYPE)),xmlhelp/util/ucpchelp1) \
    xmloff/util/xo \
))

$(eval $(call gb_CppunitTest_use_configuration,sw_odfimport))

$(eval $(call gb_CppunitTest_use_filter_configuration,sw_odfimport))

$(eval $(call gb_CppunitTest_use_unittest_configuration,sw_odfimport))

# vim: set noet sw=4 ts=4:
