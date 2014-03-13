# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,vclopengl))

$(eval $(call gb_Library_set_include,vclopengl,\
    $$(INCLUDE) \
))

$(eval $(call gb_Library_use_externals,vclopengl,\
	boost_headers \
	mdds_headers \
	glm_headers \
	mesa_headers \
	glew \
))

$(eval $(call gb_Library_use_sdk_api,vclopengl))

$(eval $(call gb_Library_use_libraries,vclopengl,\
    comphelper \
    cppu \
    cppuhelper \
    editeng \
    sal \
    vcl \
    $(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_exception_objects,vclopengl,\
    vcl/source/opengl/OpenGLRender \
))

ifeq ($(strip $(OS)),WNT)
$(eval $(call gb_Library_use_system_win32_libs,vclopengl,\
    opengl32 \
	gdi32 \
	glu32 \
))
else ifeq ($(OS),MACOSX)
$(eval $(call gb_Library_use_system_darwin_frameworks,vclopengl,\
	OpenGL \
))
else ifeq ($(OS),LINUX)
$(eval $(call gb_Library_add_libs,vclopengl,\
    -ldl \
    -lGL \
    -lGLU \
    -lX11 \
))
endif

# vim: set noet sw=4 ts=4:
