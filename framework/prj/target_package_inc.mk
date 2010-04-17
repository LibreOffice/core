#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2009 by Sun Microsystems, Inc.
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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.	If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

$(eval $(call gb_Package_Package,framework_inc,$(SRCDIR)/framework/inc))
$(eval $(call gb_Package_add_file,framework_inc,inc/framework/acceleratorinfo.hxx,helper/acceleratorinfo.hxx))
$(eval $(call gb_Package_add_file,framework_inc,inc/framework/actiontriggerhelper.hxx,helper/actiontriggerhelper.hxx))
$(eval $(call gb_Package_add_file,framework_inc,inc/framework/addonmenu.hxx,classes/addonmenu.hxx))
$(eval $(call gb_Package_add_file,framework_inc,inc/framework/addonsoptions.hxx,classes/addonsoptions.hxx))
$(eval $(call gb_Package_add_file,framework_inc,inc/framework/bmkmenu.hxx,classes/bmkmenu.hxx))
$(eval $(call gb_Package_add_file,framework_inc,inc/framework/configimporter.hxx,helper/configimporter.hxx))
$(eval $(call gb_Package_add_file,framework_inc,inc/framework/eventsconfiguration.hxx,xml/eventsconfiguration.hxx))
$(eval $(call gb_Package_add_file,framework_inc,inc/framework/imageproducer.hxx,helper/imageproducer.hxx))
$(eval $(call gb_Package_add_file,framework_inc,inc/framework/imagesconfiguration.hxx,xml/imagesconfiguration.hxx))
$(eval $(call gb_Package_add_file,framework_inc,inc/framework/interaction.hxx,dispatch/interaction.hxx))
$(eval $(call gb_Package_add_file,framework_inc,inc/framework/menuconfiguration.hxx,xml/menuconfiguration.hxx))
$(eval $(call gb_Package_add_file,framework_inc,inc/framework/menuextensionsupplier.hxx,classes/menuextensionsupplier.hxx))
$(eval $(call gb_Package_add_file,framework_inc,inc/framework/preventduplicateinteraction.hxx,interaction/preventduplicateinteraction.hxx))
$(eval $(call gb_Package_add_file,framework_inc,inc/framework/sfxhelperfunctions.hxx,classes/sfxhelperfunctions.hxx))
$(eval $(call gb_Package_add_file,framework_inc,inc/framework/titlehelper.hxx,helper/titlehelper.hxx))
$(eval $(call gb_Package_add_file,framework_inc,inc/framework/toolboxconfiguration.hxx,xml/toolboxconfiguration.hxx))
$(eval $(call gb_Package_add_file,framework_inc,inc/fwedllapi.h,fwedllapi.h))
$(eval $(call gb_Package_add_file,framework_inc,inc/fwkdllapi.h,fwkdllapi.h))
$(eval $(call gb_Package_add_file,framework_inc,inc/fwidllapi.h,fwidllapi.h))
