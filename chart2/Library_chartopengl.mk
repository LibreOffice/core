# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,chartopengl))

$(eval $(call gb_Library_set_include,chartopengl,\
    -I$(SRCDIR)/chart2/inc \
    -I$(SRCDIR)/chart2/source/model/inc \
    -I$(SRCDIR)/chart2/source/view/inc \
    -I$(SRCDIR)/chart2/source/inc \
    $$(INCLUDE) \
))

$(eval $(call gb_Library_set_precompiled_header,chartopengl,$(SRCDIR)/chart2/inc/pch/precompiled_chartopengl))

$(eval $(call gb_Library_use_externals,chartopengl,\
	boost_headers \
	glm_headers \
	$(if $(filter WNT MACOSX,$(OS)),mesa_headers) \
	glew \
))

$(eval $(call gb_Library_use_sdk_api,chartopengl))

$(eval $(call gb_Library_use_libraries,chartopengl,\
    basegfx \
    chartcore \
    comphelper \
    cppu \
    cppuhelper \
    drawinglayer \
    editeng \
    fwe \
    i18nlangtag \
    sal \
    sfx \
    svl \
    svt \
    svxcore \
    tl \
	tk \
    ucbhelper \
    utl \
    vcl \
    $(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_exception_objects,chartopengl,\
    chart2/source/view/main/OpenglShapeFactory \
    chart2/source/view/main/DummyXShape \
    chart2/source/view/main/OpenGLRender \
))
 
ifeq ($(strip $(OS)),WNT)
$(eval $(call gb_Library_use_system_win32_libs,chartopengl,\
	opengl32 \
	gdi32 \
))
else ifeq ($(OS),MACOSX)
$(eval $(call gb_Library_use_system_darwin_frameworks,chartopengl,\
	OpenGL \
))
else ifeq ($(OS),LINUX)
$(eval $(call gb_Library_add_libs,chartopengl,\
	-ldl \
	-lGL \
	-lX11 \
))
endif
 
# vim: set noet sw=4 ts=4:
