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

$(eval $(call gb_Library_Library,gdipluscanvas))

$(eval $(call gb_Library_set_componentfile,gdipluscanvas,canvas/source/directx/gdipluscanvas))

$(eval $(call gb_Library_use_external,gdipluscanvas,boost_headers))

$(eval $(call gb_Library_use_sdk_api,gdipluscanvas))

# clear NOMINMAX because GdiplusTypes.h uses those macros :(
$(eval $(call gb_Library_add_defs,gdipluscanvas,\
	-DDIRECTX_VERSION=0x0900 \
	-UNOMINMAX \
))

ifeq ($(WINDOWS_SDK_VERSION),80)
$(eval $(call gb_Library_add_defs,gdipluscanvas,\
	-DWIN8_SDK=1 \
))
endif

ifneq ($(strip $(VERBOSE)$(verbose)),)
$(eval $(call gb_Library_add_defs,gdipluscanvas,\
	-DVERBOSE \
))
endif

$(eval $(call gb_Library_use_libraries,gdipluscanvas,\
	cppu \
	tk \
	sal \
	comphelper \
	cppuhelper \
	basegfx \
	canvastools \
	vcl \
	tl \
	utl \
	i18nlangtag \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_use_system_win32_libs,gdipluscanvas,\
	gdi32 \
	gdiplus \
))

$(eval $(call gb_Library_use_static_libraries,gdipluscanvas,\
	directxcanvas \
))

$(eval $(call gb_Library_add_exception_objects,gdipluscanvas,\
	canvas/source/directx/dx_canvas \
))

# vim: set noet sw=4 ts=4:
