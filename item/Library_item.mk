# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,item))

$(eval $(call gb_Library_use_external,item,boost_headers))

$(eval $(call gb_Library_set_precompiled_header,item,$(SRCDIR)/item/inc/pch/precompiled_item))

$(eval $(call gb_Library_use_sdk_api,item))

$(eval $(call gb_Library_use_libraries,item,\
    cppu \
    cppuhelper \
    sal \
))

#$(eval $(call gb_Library_set_include,item,\
#    -I$(SRCDIR)/item/source/inc \
#    $$(INCLUDE) \
#))

$(eval $(call gb_Library_add_defs,item,\
    -DITEM_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_add_exception_objects,item,\
    item/source/base/IBase \
    item/source/base/IAdministrator \
    item/source/base/ModelSpecificIValues \
    item/source/base/ISet \
    item/source/simple/CntInt16 \
    item/source/simple/CntOUString \
))

# vim: set noet sw=4 ts=4:
