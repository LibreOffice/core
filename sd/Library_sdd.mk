# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,sdd))

$(eval $(call gb_Library_set_include,sdd,\
    $$(INCLUDE) \
    -I$(SRCDIR)/sd/inc \
))

$(eval $(call gb_Library_use_external,sdd,boost_headers))

$(eval $(call gb_Library_use_sdk_api,sdd))

$(eval $(call gb_Library_use_libraries,sdd,\
    comphelper \
    cppu \
    cppuhelper \
    sal \
    sfx \
    sot \
    tl \
    utl \
    vcl \
))

$(eval $(call gb_Library_set_componentfile,sdd,sd/util/sdd,services))

$(eval $(call gb_Library_use_common_precompiled_header,sdd))

$(eval $(call gb_Library_add_exception_objects,sdd,\
    sd/source/ui/unoidl/sddetect \
))

# vim: set noet sw=4 ts=4:
