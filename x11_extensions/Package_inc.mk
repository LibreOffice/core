# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,x11_extensions_inc,$(SRCDIR)/x11_extensions/inc))

$(eval $(call gb_Package_add_file,x11_extensions_inc,inc/X11/extensions/randr.h,randr.h))
$(eval $(call gb_Package_add_file,x11_extensions_inc,inc/X11/extensions/randrproto.h,randrproto.h))
$(eval $(call gb_Package_add_file,x11_extensions_inc,inc/X11/extensions/render.h,render.h))
$(eval $(call gb_Package_add_file,x11_extensions_inc,inc/X11/extensions/renderproto.h,renderproto.h))
$(eval $(call gb_Package_add_file,x11_extensions_inc,inc/X11/extensions/shapeconst.h,shapeconst.h))
$(eval $(call gb_Package_add_file,x11_extensions_inc,inc/X11/extensions/shape.h,shape.h))
$(eval $(call gb_Package_add_file,x11_extensions_inc,inc/X11/extensions/Xrandr.h,Xrandr.h))
$(eval $(call gb_Package_add_file,x11_extensions_inc,inc/X11/extensions/Xrender.h,Xrender.h))

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
