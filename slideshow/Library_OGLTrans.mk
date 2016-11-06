# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,OGLTrans))

ifneq ($(strip $(debug)$(DEBUG)),)
$(eval $(call gb_Library_add_defs,OGLTrans,\
    -DBOOST_SP_ENABLE_DEBUG_HOOKS \
))
endif

$(eval $(call gb_Library_add_defs,OGLTrans,\
    -DGLM_FORCE_RADIANS \
))

$(eval $(call gb_Library_use_sdk_api,OGLTrans))

$(eval $(call gb_Library_use_libraries,OGLTrans,\
	basegfx \
    canvastools \
    comphelper \
    cppu \
    cppuhelper \
    sal \
    vcl \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_use_externals,OGLTrans,\
	boost_headers \
	glew \
	glm_headers \
))

$(eval $(call gb_Library_set_componentfile,OGLTrans,slideshow/source/engine/OGLTrans/ogltrans))

ifeq ($(strip $(OS)),MACOSX)

$(eval $(call gb_Library_add_exception_objects,OGLTrans,\
    slideshow/source/engine/OGLTrans/generic/OGLTrans_TransitionerImpl \
    slideshow/source/engine/OGLTrans/generic/OGLTrans_TransitionImpl \
    slideshow/source/engine/OGLTrans/generic/OGLTrans_Operation \
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
    slideshow/source/engine/OGLTrans/generic/OGLTrans_TransitionerImpl \
    slideshow/source/engine/OGLTrans/generic/OGLTrans_TransitionImpl \
    slideshow/source/engine/OGLTrans/generic/OGLTrans_Operation \
))

else
$(eval $(call gb_Library_add_exception_objects,OGLTrans,\
    slideshow/source/engine/OGLTrans/generic/OGLTrans_TransitionerImpl \
    slideshow/source/engine/OGLTrans/generic/OGLTrans_TransitionImpl \
    slideshow/source/engine/OGLTrans/generic/OGLTrans_Operation \
))

$(eval $(call gb_Library_add_libs,OGLTrans,\
	-lGL \
	-lX11 \
))
endif

# vim: set noet sw=4 ts=4:
