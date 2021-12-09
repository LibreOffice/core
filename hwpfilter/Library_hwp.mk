# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,hwp))

$(eval $(call gb_Library_use_sdk_api,hwp))

$(eval $(call gb_Library_use_libraries,hwp,\
    comphelper \
    cppu \
    cppuhelper \
    sal \
    utl \
    tl \
))

$(eval $(call gb_Library_use_externals,hwp,\
	boost_headers \
	zlib \
))

$(eval $(call gb_Library_set_precompiled_header,hwp,hwpfilter/inc/pch/precompiled_hwp))

ifeq ($(OS),WNT)

$(eval $(call gb_Library_use_system_win32_libs,hwp,\
    ole32 \
    uuid \
))

endif

$(eval $(call gb_Library_set_componentfile,hwp,hwpfilter/source/hwp,services))

$(eval $(call gb_Library_add_exception_objects,hwp,\
    hwpfilter/source/attributes \
    hwpfilter/source/cspline \
    hwpfilter/source/fontmap \
    hwpfilter/source/formula \
    hwpfilter/source/grammar \
    hwpfilter/source/hbox \
    hwpfilter/source/hcode \
    hwpfilter/source/hfont \
    hwpfilter/source/hgzip \
    hwpfilter/source/hinfo \
    hwpfilter/source/hiodev \
    hwpfilter/source/hpara \
    hwpfilter/source/hstream \
    hwpfilter/source/hstyle \
    hwpfilter/source/htags \
    hwpfilter/source/hutil \
    hwpfilter/source/hwpeq \
    hwpfilter/source/hwpfile \
    hwpfilter/source/hwpread \
    hwpfilter/source/hwpreader \
    hwpfilter/source/lexer \
    hwpfilter/source/mzstring \
    hwpfilter/source/solver \
))

# vim: set noet sw=4 ts=4:
