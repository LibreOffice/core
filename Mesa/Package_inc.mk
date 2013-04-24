# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,Mesa_inc,$(SRCDIR)/Mesa/inc))

$(eval $(call gb_Package_add_file,Mesa_inc,inc/GL/glext.h,glext.h))
$(eval $(call gb_Package_add_file,Mesa_inc,inc/GL/gl.h,gl.h))
$(eval $(call gb_Package_add_file,Mesa_inc,inc/GL/gl_mangle.h,gl_mangle.h))
$(eval $(call gb_Package_add_file,Mesa_inc,inc/GL/glu.h,glu.h))
$(eval $(call gb_Package_add_file,Mesa_inc,inc/GL/glu_mangle.h,glu_mangle.h))
$(eval $(call gb_Package_add_file,Mesa_inc,inc/GL/glxext.h,glxext.h))
$(eval $(call gb_Package_add_file,Mesa_inc,inc/GL/glx.h,glx.h))
$(eval $(call gb_Package_add_file,Mesa_inc,inc/GL/glx_mangle.h,glx_mangle.h))
$(eval $(call gb_Package_add_file,Mesa_inc,inc/GL/vms_x_fix.h,vms_x_fix.h))
$(eval $(call gb_Package_add_file,Mesa_inc,inc/GL/wglext.h,wglext.h))

# vim: set noet sw=4:
