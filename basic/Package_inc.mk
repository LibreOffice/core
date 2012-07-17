# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,basic_inc,$(SRCDIR)/basic/inc))

$(eval $(call gb_Package_add_file,basic_inc,inc/basic/basicdllapi.h,basic/basicdllapi.h))
$(eval $(call gb_Package_add_file,basic_inc,inc/basic/basicmanagerrepository.hxx,basic/basicmanagerrepository.hxx))
$(eval $(call gb_Package_add_file,basic_inc,inc/basic/basmgr.hxx,basic/basmgr.hxx))
$(eval $(call gb_Package_add_file,basic_inc,inc/basic/basrdll.hxx,basic/basrdll.hxx))
$(eval $(call gb_Package_add_file,basic_inc,inc/basic/modsizeexceeded.hxx,basic/modsizeexceeded.hxx))
$(eval $(call gb_Package_add_file,basic_inc,inc/basic/sbdef.hxx,basic/sbdef.hxx))
$(eval $(call gb_Package_add_file,basic_inc,inc/basic/sberrors.hxx,basic/sberrors.hxx))
$(eval $(call gb_Package_add_file,basic_inc,inc/basic/sbmeth.hxx,basic/sbmeth.hxx))
$(eval $(call gb_Package_add_file,basic_inc,inc/basic/sbmod.hxx,basic/sbmod.hxx))
$(eval $(call gb_Package_add_file,basic_inc,inc/basic/sbstar.hxx,basic/sbstar.hxx))
$(eval $(call gb_Package_add_file,basic_inc,inc/basic/sbuno.hxx,basic/sbuno.hxx))
$(eval $(call gb_Package_add_file,basic_inc,inc/basic/sbxcore.hxx,basic/sbxcore.hxx))
$(eval $(call gb_Package_add_file,basic_inc,inc/basic/sbxdef.hxx,basic/sbxdef.hxx))
$(eval $(call gb_Package_add_file,basic_inc,inc/basic/sbxfac.hxx,basic/sbxfac.hxx))
$(eval $(call gb_Package_add_file,basic_inc,inc/basic/sbxform.hxx,basic/sbxform.hxx))
$(eval $(call gb_Package_add_file,basic_inc,inc/basic/sbx.hxx,basic/sbx.hxx))
$(eval $(call gb_Package_add_file,basic_inc,inc/basic/global.hxx,basic/global.hxx))
$(eval $(call gb_Package_add_file,basic_inc,inc/basic/sbxmeth.hxx,basic/sbxmeth.hxx))
$(eval $(call gb_Package_add_file,basic_inc,inc/basic/sbxobj.hxx,basic/sbxobj.hxx))
$(eval $(call gb_Package_add_file,basic_inc,inc/basic/sbxprop.hxx,basic/sbxprop.hxx))
$(eval $(call gb_Package_add_file,basic_inc,inc/basic/sbxvar.hxx,basic/sbxvar.hxx))
$(eval $(call gb_Package_add_file,basic_inc,inc/basic/vbahelper.hxx,basic/vbahelper.hxx))

# vim: set noet sw=4 ts=4:
