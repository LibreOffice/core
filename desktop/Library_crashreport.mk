# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,crashreport))

$(eval $(call gb_Library_set_include,crashreport,\
    $$(INCLUDE) \
    -I$(SRCDIR)/desktop/inc \
))

$(eval $(call gb_Library_use_externals,crashreport,\
    breakpad \
    curl \
))

$(eval $(call gb_Library_add_defs,crashreport,\
    -DCRASHREPORT_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_add_libs,crashreport,\
    $(if $(filter LINUX %BSD SOLARIS, $(OS)), \
        $(DLOPEN_LIBS) \
    ) \
))

$(eval $(call gb_Library_use_sdk_api,crashreport))

$(eval $(call gb_Library_use_libraries,crashreport,\
    comphelper \
    cppu \
    cppuhelper \
    sal \
    salhelper \
    ucbhelper \
    utl \
))

$(eval $(call gb_Library_add_exception_objects,crashreport,\
    desktop/source/app/crashreport \
    desktop/source/minidump/minidump \
))


# vim: set ts=4 sw=4 et:
