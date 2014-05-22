# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,OGLTrans))

ifeq ($(strip $(VERBOSE)),TRUE)
$(eval $(call gb_Library_add_defs,OGLTrans,\
    -DVERBOSE \
))
endif

$(eval $(call gb_Library_use_sdk_api,OGLTrans))

$(eval $(call gb_Library_use_libraries,OGLTrans,\
	basegfx \
    canvastools \
    comphelper \
    cppu \
    cppuhelper \
    sal \
    vcl \
    vclopengl \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_use_externals,OGLTrans,\
	boost_headers \
	glew \
	mesa_headers \
))

$(eval $(call gb_Library_set_componentfile,OGLTrans,slideshow/source/engine/OGLTrans/ogltrans))

ifeq ($(strip $(OS)),MACOSX)
$(eval $(call gb_Library_add_objcobjects,OGLTrans,\
    slideshow/source/engine/OGLTrans/mac/aquaOpenGLView \
    slideshow/source/engine/OGLTrans/mac/OGLTrans_Shaders \
    ,\
))

$(eval $(call gb_Library_add_objcxxobjects,OGLTrans,\
    slideshow/source/engine/OGLTrans/mac/OGLTrans_TransitionerImpl \
    slideshow/source/engine/OGLTrans/mac/OGLTrans_TransitionImpl \
    , $(gb_LinkTarget_EXCEPTIONFLAGS) \
))

$(eval $(call gb_Library_use_system_darwin_frameworks,OGLTrans,\
    Cocoa \
    GLUT \
    OpenGL \
))

else ifeq ($(strip $(OS)),WNT)
$(eval $(call gb_Library_use_system_win32_libs,OGLTrans,\
    gdi32 \
    glu32 \
    opengl32 \
))

$(eval $(call gb_Library_add_exception_objects,OGLTrans,\
    slideshow/source/engine/OGLTrans/generic/OGLTrans_Shaders \
    slideshow/source/engine/OGLTrans/generic/OGLTrans_TransitionerImpl \
    slideshow/source/engine/OGLTrans/generic/OGLTrans_TransitionImpl \
))

else
$(eval $(call gb_Library_add_exception_objects,OGLTrans,\
    slideshow/source/engine/OGLTrans/generic/OGLTrans_Shaders \
    slideshow/source/engine/OGLTrans/generic/OGLTrans_TransitionerImpl \
    slideshow/source/engine/OGLTrans/generic/OGLTrans_TransitionImpl \
))

$(eval $(call gb_Library_add_libs,OGLTrans,\
	-lGL \
	-lGLU \
	-lX11 \
))
endif

# vim: set noet sw=4 ts=4:
