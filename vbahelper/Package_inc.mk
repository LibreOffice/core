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

$(eval $(call gb_Package_Package,vbahelper_inc,$(SRCDIR)/vbahelper/inc))
$(eval $(call gb_Package_add_file,vbahelper_inc,inc/vbahelper/collectionbase.hxx,vbahelper/collectionbase.hxx))
$(eval $(call gb_Package_add_file,vbahelper_inc,inc/vbahelper/helperdecl.hxx,vbahelper/helperdecl.hxx))
$(eval $(call gb_Package_add_file,vbahelper_inc,inc/vbahelper/vbaaccesshelper.hxx,vbahelper/vbaaccesshelper.hxx))
$(eval $(call gb_Package_add_file,vbahelper_inc,inc/vbahelper/vbaapplicationbase.hxx,vbahelper/vbaapplicationbase.hxx))
$(eval $(call gb_Package_add_file,vbahelper_inc,inc/vbahelper/vbacollectionimpl.hxx,vbahelper/vbacollectionimpl.hxx))
$(eval $(call gb_Package_add_file,vbahelper_inc,inc/vbahelper/vbadialogbase.hxx,vbahelper/vbadialogbase.hxx))
$(eval $(call gb_Package_add_file,vbahelper_inc,inc/vbahelper/vbadialogsbase.hxx,vbahelper/vbadialogsbase.hxx))
$(eval $(call gb_Package_add_file,vbahelper_inc,inc/vbahelper/vbadllapi.h,vbahelper/vbadllapi.h))
$(eval $(call gb_Package_add_file,vbahelper_inc,inc/vbahelper/vbadocumentbase.hxx,vbahelper/vbadocumentbase.hxx))
$(eval $(call gb_Package_add_file,vbahelper_inc,inc/vbahelper/vbadocumentsbase.hxx,vbahelper/vbadocumentsbase.hxx))
$(eval $(call gb_Package_add_file,vbahelper_inc,inc/vbahelper/vbaeventshelperbase.hxx,vbahelper/vbaeventshelperbase.hxx))
$(eval $(call gb_Package_add_file,vbahelper_inc,inc/vbahelper/vbafontbase.hxx,vbahelper/vbafontbase.hxx))
$(eval $(call gb_Package_add_file,vbahelper_inc,inc/vbahelper/vbaglobalbase.hxx,vbahelper/vbaglobalbase.hxx))
$(eval $(call gb_Package_add_file,vbahelper_inc,inc/vbahelper/vbahelper.hxx,vbahelper/vbahelper.hxx))
$(eval $(call gb_Package_add_file,vbahelper_inc,inc/vbahelper/vbahelperinterface.hxx,vbahelper/vbahelperinterface.hxx))
$(eval $(call gb_Package_add_file,vbahelper_inc,inc/vbahelper/vbapagesetupbase.hxx,vbahelper/vbapagesetupbase.hxx))
$(eval $(call gb_Package_add_file,vbahelper_inc,inc/vbahelper/vbapropvalue.hxx,vbahelper/vbapropvalue.hxx))
$(eval $(call gb_Package_add_file,vbahelper_inc,inc/vbahelper/vbashape.hxx,vbahelper/vbashape.hxx))
$(eval $(call gb_Package_add_file,vbahelper_inc,inc/vbahelper/vbashaperange.hxx,vbahelper/vbashaperange.hxx))
$(eval $(call gb_Package_add_file,vbahelper_inc,inc/vbahelper/vbashapes.hxx,vbahelper/vbashapes.hxx))
$(eval $(call gb_Package_add_file,vbahelper_inc,inc/vbahelper/vbatextframe.hxx,vbahelper/vbatextframe.hxx))
$(eval $(call gb_Package_add_file,vbahelper_inc,inc/vbahelper/vbawindowbase.hxx,vbahelper/vbawindowbase.hxx))
$(eval $(call gb_Package_add_file,vbahelper_inc,inc/vbahelper/weakreference.hxx,vbahelper/weakreference.hxx))

# vim: set noet sw=4 ts=4:
