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

$(eval $(call gb_Library_Library,vclcanvas))

$(eval $(call gb_Library_set_componentfile,vclcanvas,canvas/source/vcl/vclcanvas))

$(eval $(call gb_Library_use_externals,vclcanvas,\
	boost_headers \
	epoxy \
))

$(eval $(call gb_Library_use_sdk_api,vclcanvas))

$(eval $(call gb_Library_use_libraries,vclcanvas,\
	basegfx \
	canvastools \
	comphelper \
	cppu \
	cppuhelper \
	i18nlangtag \
	sal \
	svt \
	tk \
	tl \
	vcl \
))

$(eval $(call gb_Library_add_exception_objects,vclcanvas,\
	canvas/source/vcl/backbuffer \
	canvas/source/vcl/bitmapbackbuffer \
	canvas/source/vcl/cachedbitmap \
	canvas/source/vcl/canvas \
	canvas/source/vcl/canvasbitmap \
	canvas/source/vcl/canvasbitmaphelper \
	canvas/source/vcl/canvascustomsprite \
	canvas/source/vcl/canvasfont \
	canvas/source/vcl/canvashelper \
	canvas/source/vcl/devicehelper \
	canvas/source/vcl/impltools \
	canvas/source/vcl/services \
	canvas/source/vcl/spritecanvas \
	canvas/source/vcl/spritecanvashelper \
	canvas/source/vcl/spritedevicehelper \
	canvas/source/vcl/spritehelper \
	canvas/source/vcl/textlayout \
	canvas/source/vcl/canvashelper_texturefill \
	canvas/source/vcl/windowoutdevholder \
))

# vim: set noet sw=4 ts=4:
