#
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
#  The contents of this file are subject to the Mozilla Public License Version
#  1.1 (the "License"); you may not use this file except in compliance with
#  the License or as specified alternatively below. You may obtain a copy of
#  the License at http://www.mozilla.org/MPL/
#
#  Software distributed under the License is distributed on an "AS IS" basis,
#  WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
#  for the specific language governing rights and limitations under the
#  License.
#
#  Major Contributor(s):
#  Copyright (C) 2011 Peter Foley <pefoley2@verizon.net> (initial developer)
#
#  All Rights Reserved.
#
#  For minor contributions see the git repository.
#
#  Alternatively, the contents of this file may be used under the terms of
#  either the GNU General Public License Version 3 or later (the "GPLv3+"), or
#  the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
#  in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
#  instead of those above.

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
