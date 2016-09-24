# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,pixelctl))

$(eval $(call gb_Executable_use_api,pixelctl,\
    offapi \
    udkapi \
))

$(eval $(call gb_Executable_use_external,pixelctl,boost_headers))

$(eval $(call gb_Executable_set_include,pixelctl,\
    $$(INCLUDE) \
    -I$(SRCDIR)/vcl/inc \
    -I$(SRCDIR)/svx/inc \
))

$(eval $(call gb_Executable_use_libraries,pixelctl,\
    svx \
    tl \
    sal \
    vcl \
    cppu \
    cppuhelper \
    comphelper \
))

$(eval $(call gb_Executable_add_exception_objects,pixelctl,\
    svx/workben/pixelctl \
))

# vim: set noet sw=4 ts=4:
