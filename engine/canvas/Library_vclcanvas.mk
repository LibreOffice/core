# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
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

$(eval $(call gb_Library_Library,vclcanvas))

$(eval $(call gb_Library_set_include,vclcanvas,\
    $$(INCLUDE) \
    -I$(SRCDIR)/canvas/inc \
))

$(eval $(call gb_Library_set_componentfile,vclcanvas,canvas/source/factory/canvasfactory,services))

$(eval $(call gb_Library_set_precompiled_header,vclcanvas,canvas/inc/pch/precompiled_vclcanvas))

$(eval $(call gb_Library_use_external,vclcanvas,boost_headers))

$(eval $(call gb_Library_use_sdk_api,vclcanvas))

$(eval $(call gb_Library_add_defs,vclcanvas,\
	-DVCLCANVAS_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_use_libraries,vclcanvas,\
	basegfx \
	canvastools \
	comphelper \
	cppu \
	cppuhelper \
	i18nlangtag \
	sal \
	salhelper \
	svt \
	tk \
	tl \
	vcl \
))

$(eval $(call gb_Library_add_exception_objects,vclcanvas,\
	canvas/source/factory/cf_service \
	canvas/source/vcl/bitmapbackbuffer \
	canvas/source/vcl/cachedbitmap \
	canvas/source/vcl/canvas \
	canvas/source/vcl/canvasbitmap \
	canvas/source/vcl/canvasbitmaphelper \
	canvas/source/vcl/canvasfont \
	canvas/source/vcl/canvashelper \
	canvas/source/vcl/devicehelper \
	canvas/source/vcl/impltools \
	canvas/source/vcl/textlayout \
	canvas/source/vcl/canvashelper_texturefill \
	canvas/source/cppcanvas/bitmapaction \
	canvas/source/cppcanvas/cachedprimitivebase \
	canvas/source/cppcanvas/implrenderer \
	canvas/source/cppcanvas/lineaction \
	canvas/source/cppcanvas/mtftools \
	canvas/source/cppcanvas/pointaction \
	canvas/source/cppcanvas/polypolyaction \
	canvas/source/cppcanvas/textaction \
	canvas/source/cppcanvas/textlineshelper \
	canvas/source/cppcanvas/transparencygroupaction \
	canvas/source/cppcanvas/tools \
	canvas/source/cppcanvas/uno_mtfrenderer \
	canvas/source/cppcanvas/implcanvas \
	canvas/source/cppcanvas/vclfactory \
))

# vim: set noet sw=4 ts=4:
