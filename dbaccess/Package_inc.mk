# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,dbaccess_inc,$(SRCDIR)/dbaccess/inc))

$(eval $(call gb_Package_add_file,dbaccess_inc,inc/dbaccess/AsyncronousLink.hxx,AsyncronousLink.hxx))
$(eval $(call gb_Package_add_file,dbaccess_inc,inc/dbaccess/controllerframe.hxx,controllerframe.hxx))
$(eval $(call gb_Package_add_file,dbaccess_inc,inc/dbaccess/dataview.hxx,dataview.hxx))
$(eval $(call gb_Package_add_file,dbaccess_inc,inc/dbaccess/dbaccessdllapi.h,dbaccessdllapi.h))
$(eval $(call gb_Package_add_file,dbaccess_inc,inc/dbaccess/dbaundomanager.hxx,dbaundomanager.hxx))
$(eval $(call gb_Package_add_file,dbaccess_inc,inc/dbaccess/dbsubcomponentcontroller.hxx,dbsubcomponentcontroller.hxx))
$(eval $(call gb_Package_add_file,dbaccess_inc,inc/dbaccess/genericcontroller.hxx,genericcontroller.hxx))
$(eval $(call gb_Package_add_file,dbaccess_inc,inc/dbaccess/IController.hxx,IController.hxx))
$(eval $(call gb_Package_add_file,dbaccess_inc,inc/dbaccess/IReference.hxx,IReference.hxx))
$(eval $(call gb_Package_add_file,dbaccess_inc,inc/dbaccess/ToolBoxHelper.hxx,ToolBoxHelper.hxx))

# vim: set noet sw=4 ts=4:
