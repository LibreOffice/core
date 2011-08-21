# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2000, 2011 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

$(eval $(call gb_Package_Package,basic_inc,$(SRCDIR)/basic/inc))

$(eval $(call gb_Package_add_file,basic_inc,inc/basic/basicdllapi.h,basic/basicdllapi.h))
$(eval $(call gb_Package_add_file,basic_inc,inc/basic/basicmanagerrepository.hxx,basic/basicmanagerrepository.hxx))
$(eval $(call gb_Package_add_file,basic_inc,inc/basic/basicrt.hxx,basic/basicrt.hxx))
$(eval $(call gb_Package_add_file,basic_inc,inc/basic/basmgr.hxx,basic/basmgr.hxx))
$(eval $(call gb_Package_add_file,basic_inc,inc/basic/basrdll.hxx,basic/basrdll.hxx))
$(eval $(call gb_Package_add_file,basic_inc,inc/basic/dispdefs.hxx,basic/dispdefs.hxx))
$(eval $(call gb_Package_add_file,basic_inc,inc/basic/modsizeexceeded.hxx,basic/modsizeexceeded.hxx))
$(eval $(call gb_Package_add_file,basic_inc,inc/basic/mybasic.hxx,basic/mybasic.hxx))
$(eval $(call gb_Package_add_file,basic_inc,inc/basic/process.hxx,basic/process.hxx))
$(eval $(call gb_Package_add_file,basic_inc,inc/basic/sbdef.hxx,basic/sbdef.hxx))
$(eval $(call gb_Package_add_file,basic_inc,inc/basic/sberrors.hxx,basic/sberrors.hxx))
$(eval $(call gb_Package_add_file,basic_inc,inc/basic/sbmeth.hxx,basic/sbmeth.hxx))
$(eval $(call gb_Package_add_file,basic_inc,inc/basic/sbmod.hxx,basic/sbmod.hxx))
$(eval $(call gb_Package_add_file,basic_inc,inc/basic/sbobjmod.hxx,basic/sbobjmod.hxx))
$(eval $(call gb_Package_add_file,basic_inc,inc/basic/sbprop.hxx,basic/sbprop.hxx))
$(eval $(call gb_Package_add_file,basic_inc,inc/basic/sbstar.hxx,basic/sbstar.hxx))
$(eval $(call gb_Package_add_file,basic_inc,inc/basic/sbstdobj.hxx,basic/sbstdobj.hxx))
$(eval $(call gb_Package_add_file,basic_inc,inc/basic/sbuno.hxx,basic/sbuno.hxx))
$(eval $(call gb_Package_add_file,basic_inc,inc/basic/sbxbase.hxx,basic/sbxbase.hxx))
$(eval $(call gb_Package_add_file,basic_inc,inc/basic/sbxcore.hxx,basic/sbxcore.hxx))
$(eval $(call gb_Package_add_file,basic_inc,inc/basic/sbxdef.hxx,basic/sbxdef.hxx))
$(eval $(call gb_Package_add_file,basic_inc,inc/basic/sbxfac.hxx,basic/sbxfac.hxx))
$(eval $(call gb_Package_add_file,basic_inc,inc/basic/sbxform.hxx,basic/sbxform.hxx))
$(eval $(call gb_Package_add_file,basic_inc,inc/basic/sbx.hxx,basic/sbx.hxx))
$(eval $(call gb_Package_add_file,basic_inc,inc/basic/sbxmeth.hxx,basic/sbxmeth.hxx))
$(eval $(call gb_Package_add_file,basic_inc,inc/basic/sbxmstrm.hxx,basic/sbxmstrm.hxx))
$(eval $(call gb_Package_add_file,basic_inc,inc/basic/sbxobj.hxx,basic/sbxobj.hxx))
$(eval $(call gb_Package_add_file,basic_inc,inc/basic/sbxprop.hxx,basic/sbxprop.hxx))
$(eval $(call gb_Package_add_file,basic_inc,inc/basic/sbxvar.hxx,basic/sbxvar.hxx))
$(eval $(call gb_Package_add_file,basic_inc,inc/basic/svtmsg.hrc,basic/svtmsg.hrc))
$(eval $(call gb_Package_add_file,basic_inc,inc/basic/testtool.hrc,basic/testtool.hrc))
$(eval $(call gb_Package_add_file,basic_inc,inc/basic/testtool.hxx,basic/testtool.hxx))
$(eval $(call gb_Package_add_file,basic_inc,inc/basic/ttglobal.hrc,basic/ttglobal.hrc))
$(eval $(call gb_Package_add_file,basic_inc,inc/basic/ttmsg.hrc,basic/ttmsg.hrc))
$(eval $(call gb_Package_add_file,basic_inc,inc/basic/ttstrhlp.hxx,basic/ttstrhlp.hxx))
$(eval $(call gb_Package_add_file,basic_inc,inc/basic/vbahelper.hxx,basic/vbahelper.hxx))

# vim: set noet sw=4 ts=4:
