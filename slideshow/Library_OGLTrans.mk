# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Initial Developer of the Original Code is
#       David Tardon, Red Hat Inc. <dtardon@redhat.com>
# Portions created by the Initial Developer are Copyright (C) 2010 the
# Initial Developer. All Rights Reserved.
#
# Major Contributor(s):
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_Library_Library,OGLTrans))

$(eval $(call gb_Library_add_precompiled_header,OGLTrans,$(SRCDIR)/sd/inc/pch/precompiled_sd))

$(eval $(call gb_Library_set_include,OGLTrans,\
    $$(INCLUDE) \
    -I$(realpath $(SRCDIR)/slideshow/inc/pch) \
    -I$(OUTDIR)/inc \
    -I$(OUTDIR)/inc/offuh \
))

ifeq ($(strip $(VERBOSE)),TRUE)
$(eval $(call gb_Library_set_defs,OGLTrans,\
    $$(DEFS) \
    -DVERBOSE \
))
endif

ifneq ($(strip $(debug)$(DEBUG)),)
$(eval $(call gb_Library_set_defs,OGLTrans,\
    $$(DEFS) \
    -DBOOST_SP_ENABLE_DEBUG_HOOKS \
))
endif

$(eval $(call gb_Library_add_linked_libs,OGLTrans,\
    canvastools \
    comphelper \
    cppu \
    cppuhelper \
    sal \
    vcl \
    $(gb_STDLIBS) \
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

$(eval $(call gb_Library_add_linked_libs,OGLTrans,\
    Cocoa \
    GLUT \
    OpenGL \
))

else ifeq ($(strip $(OS)),WNT)
$(eval $(call gb_Library_add_linked_libs,OGLTrans,\
    gdi32 \
    glu32 \
    opengl32 \
))

$(eval $(call gb_Library_add_exception_objects,OGLTrans,\
    slideshow/source/engine/OGLTrans/win/OGLTrans_Shaders \
    slideshow/source/engine/OGLTrans/win/OGLTrans_TransitionerImpl \
    slideshow/source/engine/OGLTrans/win/OGLTrans_TransitionImpl \
))

else
$(eval $(call gb_Library_add_exception_objects,OGLTrans,\
    slideshow/source/engine/OGLTrans/unx/OGLTrans_Shaders \
    slideshow/source/engine/OGLTrans/unx/OGLTrans_TransitionerImpl \
    slideshow/source/engine/OGLTrans/unx/OGLTrans_TransitionImpl \
))

$(eval $(call gb_Library_add_linked_libs,OGLTrans,\
    GL \
    GLU \
    X11 \
))
endif

# vim: set noet ts=4 sw=4:
