# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,vcldemo))

$(eval $(call gb_Executable_use_api,vcldemo,\
    offapi \
    udkapi \
))

ifeq ($(DISABLE_GUI),)
$(eval $(call gb_Executable_use_externals,vcldemo,\
    epoxy \
))
endif

$(eval $(call gb_Executable_add_defs,vcldemo,\
    -DVCL_INTERNALS \
))

$(eval $(call gb_Executable_set_include,vcldemo,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
))

$(eval $(call gb_Executable_use_libraries,vcldemo,\
    basegfx \
    comphelper \
    cppu \
    cppuhelper \
    tl \
    sal \
    salhelper \
    fwk \
    i18nlangtag \
    vcl \
))

$(eval $(call gb_Executable_use_vclmain,vcldemo))

$(eval $(call gb_Executable_add_exception_objects,vcldemo,\
    vcl/workben/vcldemo \
))

# vim: set noet sw=4 ts=4:
