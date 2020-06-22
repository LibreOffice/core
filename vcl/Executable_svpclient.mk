# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,svpclient))

$(eval $(call gb_Executable_use_api,svpclient,\
    offapi \
    udkapi \
))

$(eval $(call gb_Executable_use_external,svpclient,boost_headers))

$(eval $(call gb_Executable_set_include,svpclient,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
))

$(eval $(call gb_Executable_add_defs,svpclient,\
    -DVCL_INTERNALS \
))

$(eval $(call gb_Executable_use_libraries,svpclient,\
    tl \
    sal \
    vcl \
    cppu \
    cppuhelper \
    comphelper \
))

ifeq ($(OS),HAIKU)
$(eval $(call gb_Executable_add_libs,svpclient,-lnetwork))
endif

$(eval $(call gb_Executable_add_exception_objects,svpclient,\
    vcl/workben/svpclient \
))

# vim: set noet sw=4 ts=4:
