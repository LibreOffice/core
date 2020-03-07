# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,drawinglayercore))

$(eval $(call gb_Library_set_include,drawinglayercore,\
    $$(INCLUDE) \
    -I$(SRCDIR)/drawinglayer/inc \
))

$(eval $(call gb_Library_add_defs,drawinglayercore,\
    -DDRAWINGLAYERCORE_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_set_precompiled_header,drawinglayercore,drawinglayer/inc/pch/precompiled_drawinglayercore))

$(eval $(call gb_Library_use_sdk_api,drawinglayercore))

$(eval $(call gb_Library_use_externals,drawinglayercore,\
    boost_headers \
    libxml2 \
))

$(eval $(call gb_Library_use_libraries,drawinglayercore,\
    basegfx \
    comphelper \
    cppu \
    cppuhelper \
    i18nlangtag \
    sal \
    salhelper \
    svl \
    tl \
))

$(eval $(call gb_Library_add_exception_objects,drawinglayercore,\
    drawinglayer/source/primitive2d/baseprimitive2d \
    drawinglayer/source/primitive2d/Primitive2DContainer \
    drawinglayer/source/primitive2d/Tools \
    drawinglayer/source/geometry/viewinformation2d \
))

# vim: set noet sw=4 ts=4:
