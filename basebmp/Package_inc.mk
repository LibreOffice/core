# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,basebmp_inc,$(SRCDIR)/basebmp/inc))

$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/bitmapdevice.hxx,basebmp/bitmapdevice.hxx))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/basebmpdllapi.h,basebmp/basebmpdllapi.h))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/color.hxx,basebmp/color.hxx))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/debug.hxx,basebmp/debug.hxx))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/drawmodes.hxx,basebmp/drawmodes.hxx))
$(eval $(call gb_Package_add_file,basebmp_inc,inc/basebmp/scanlineformats.hxx,basebmp/scanlineformats.hxx))

# vim: set noet sw=4 ts=4:
