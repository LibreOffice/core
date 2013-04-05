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

$(eval $(call gb_Library_Library,cppcanvas))

$(eval $(call gb_Library_use_package,cppcanvas,cppcanvas_inc))

$(eval $(call gb_Library_set_include,cppcanvas,\
	-I$(SRCDIR)/cppcanvas/inc \
	-I$(SRCDIR)/cppcanvas/source/inc \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_use_sdk_api,cppcanvas))
$(eval $(call gb_Library_add_defs,cppcanvas,\
	-DCPPCANVAS_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_use_external,cppcanvas,boost_headers))

$(eval $(call gb_Library_use_libraries,cppcanvas,\
	basegfx \
	canvastools \
	comphelper \
	cppu \
	cppuhelper \
	i18nlangtag \
	sal \
	svt \
	tl \
	vcl \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_exception_objects,cppcanvas,\
	cppcanvas/source/mtfrenderer/bitmapaction \
	cppcanvas/source/mtfrenderer/cachedprimitivebase \
	cppcanvas/source/mtfrenderer/emfplus \
	cppcanvas/source/mtfrenderer/implrenderer \
	cppcanvas/source/mtfrenderer/lineaction \
	cppcanvas/source/mtfrenderer/mtftools \
	cppcanvas/source/mtfrenderer/pointaction \
	cppcanvas/source/mtfrenderer/polypolyaction \
	cppcanvas/source/mtfrenderer/textaction \
	cppcanvas/source/mtfrenderer/transparencygroupaction \
	cppcanvas/source/tools/canvasgraphichelper \
	cppcanvas/source/tools/tools \
	cppcanvas/source/wrapper/basegfxfactory \
	cppcanvas/source/wrapper/implbitmap \
	cppcanvas/source/wrapper/implbitmapcanvas \
	cppcanvas/source/wrapper/implcanvas \
	cppcanvas/source/wrapper/implcolor \
	cppcanvas/source/wrapper/implcustomsprite \
	cppcanvas/source/wrapper/implfont \
	cppcanvas/source/wrapper/implpolypolygon \
	cppcanvas/source/wrapper/implsprite \
	cppcanvas/source/wrapper/implspritecanvas \
	cppcanvas/source/wrapper/vclfactory \
))

# vim: set noet sw=4 ts=4:
