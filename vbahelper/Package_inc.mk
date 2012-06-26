# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

$(eval $(call gb_Package_Package,vbahelper_inc,$(SRCDIR)/vbahelper/inc))
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

# vim: set noet sw=4 ts=4:
