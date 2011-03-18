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

$(eval $(call gb_Package_Package,toolkit_source,$(SRCDIR)/toolkit/source))
$(eval $(call gb_Package_add_file,toolkit_source,inc/layout/core/bin.hxx,layout/core/bin.hxx))
$(eval $(call gb_Package_add_file,toolkit_source,inc/layout/core/box-base.hxx,layout/core/box-base.hxx))
$(eval $(call gb_Package_add_file,toolkit_source,inc/layout/core/box.hxx,layout/core/box.hxx))
$(eval $(call gb_Package_add_file,toolkit_source,inc/layout/core/container.hxx,layout/core/container.hxx))
$(eval $(call gb_Package_add_file,toolkit_source,inc/layout/core/dialogbuttonhbox.hxx,layout/core/dialogbuttonhbox.hxx))
$(eval $(call gb_Package_add_file,toolkit_source,inc/layout/core/factory.hxx,layout/core/factory.hxx))
$(eval $(call gb_Package_add_file,toolkit_source,inc/layout/core/flow.hxx,layout/core/flow.hxx))
$(eval $(call gb_Package_add_file,toolkit_source,inc/layout/core/helper.hxx,layout/core/helper.hxx))
$(eval $(call gb_Package_add_file,toolkit_source,inc/layout/core/import.hxx,layout/core/import.hxx))
$(eval $(call gb_Package_add_file,toolkit_source,inc/layout/core/localized-string.hxx,layout/core/localized-string.hxx))
$(eval $(call gb_Package_add_file,toolkit_source,inc/layout/core/precompiled_xmlscript.hxx,layout/core/precompiled_xmlscript.hxx))
$(eval $(call gb_Package_add_file,toolkit_source,inc/layout/core/proplist.hxx,layout/core/proplist.hxx))
$(eval $(call gb_Package_add_file,toolkit_source,inc/layout/core/root.hxx,layout/core/root.hxx))
$(eval $(call gb_Package_add_file,toolkit_source,inc/layout/core/table.hxx,layout/core/table.hxx))
$(eval $(call gb_Package_add_file,toolkit_source,inc/layout/core/timer.hxx,layout/core/timer.hxx))
$(eval $(call gb_Package_add_file,toolkit_source,inc/layout/core/translate.hxx,layout/core/translate.hxx))
$(eval $(call gb_Package_add_file,toolkit_source,inc/layout/core/vcl.hxx,layout/core/vcl.hxx))
$(eval $(call gb_Package_add_file,toolkit_source,inc/layout/vcl/wrapper.hxx,layout/vcl/wrapper.hxx))
$(eval $(call gb_Package_add_file,toolkit_source,inc/toolkit/awt/vclxdialog.hxx,awt/vclxdialog.hxx))
