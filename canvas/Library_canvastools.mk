# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

$(eval $(call gb_Library_Library,canvastools))

$(eval $(call gb_Library_set_include,canvastools,\
    $$(INCLUDE) \
    -I$(SRCDIR)/canvas/inc \
))

$(eval $(call gb_Library_set_precompiled_header,canvastools,canvas/inc/pch/precompiled_canvastools))

$(eval $(call gb_Library_use_sdk_api,canvastools))

$(eval $(call gb_Library_add_defs,canvastools,\
	-DCANVASTOOLS_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_use_external,canvastools,boost_headers))

$(eval $(call gb_Library_use_libraries,canvastools,\
	basegfx \
	comphelper \
	cppu \
	cppuhelper \
	sal \
	tk \
	tl \
	vcl \
))

$(eval $(call gb_Library_add_exception_objects,canvastools,\
	canvas/source/tools/cachedprimitivebase \
	canvas/source/tools/canvascustomspritehelper \
	canvas/source/tools/canvastools \
	canvas/source/tools/elapsedtime \
	canvas/source/tools/page \
	canvas/source/tools/pagemanager \
	canvas/source/tools/parametricpolypolygon \
	canvas/source/tools/propertysethelper \
	canvas/source/tools/spriteredrawmanager \
	canvas/source/tools/surface \
	canvas/source/tools/surfaceproxy \
	canvas/source/tools/surfaceproxymanager \
	canvas/source/tools/verifyinput \
))

ifeq ($(strip $(ENABLE_AGG)),YES)
$(eval $(call gb_Library_add_exception_objects,canvastools,\
	canvas/source/tools/bitmap \
	canvas/source/tools/image \
))
$(eval $(call gb_Library_use_libraries,canvastools,\
	agg \
))
ifneq ($(strip $(AGG_VERSION)),)
$(eval $(call gb_Library_add_defs,canvastools,\
	-DAGG_VERSION=$(AGG_VERSION) \
))
endif
endif

# vim: set noet sw=4 ts=4:
