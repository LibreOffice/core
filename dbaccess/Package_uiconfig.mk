# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Initial Developer of the Original Code is
#       David Tardon, Red Hat Inc. <dtardon@redhat.com>
# Portions created by the Initial Developer are Copyright (C) 2010 the
# Initial Developer. All Rights Reserved.
#
# Major Contributor(s):
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_Package_Package,dbaccess_uiconfig,$(SRCDIR)/dbaccess/uiconfig))

$(eval $(call gb_Package_add_file,dbaccess_uiconfig,xml/uiconfig/modules/dbapp/menubar/menubar.xml,dbapp/menubar/menubar.xml))
$(eval $(call gb_Package_add_file,dbaccess_uiconfig,xml/uiconfig/modules/dbapp/statusbar/statusbar.xml,dbapp/statusbar/statusbar.xml))
$(eval $(call gb_Package_add_file,dbaccess_uiconfig,xml/uiconfig/modules/dbapp/toolbar/formobjectbar.xml,dbapp/toolbar/formobjectbar.xml))
$(eval $(call gb_Package_add_file,dbaccess_uiconfig,xml/uiconfig/modules/dbapp/toolbar/queryobjectbar.xml,dbapp/toolbar/queryobjectbar.xml))
$(eval $(call gb_Package_add_file,dbaccess_uiconfig,xml/uiconfig/modules/dbapp/toolbar/reportobjectbar.xml,dbapp/toolbar/reportobjectbar.xml))
$(eval $(call gb_Package_add_file,dbaccess_uiconfig,xml/uiconfig/modules/dbapp/toolbar/tableobjectbar.xml,dbapp/toolbar/tableobjectbar.xml))
$(eval $(call gb_Package_add_file,dbaccess_uiconfig,xml/uiconfig/modules/dbapp/toolbar/toolbar.xml,dbapp/toolbar/toolbar.xml))
$(eval $(call gb_Package_add_file,dbaccess_uiconfig,xml/uiconfig/modules/dbbrowser/menubar/compat.xml,dbbrowser/menubar/compat.xml))
$(eval $(call gb_Package_add_file,dbaccess_uiconfig,xml/uiconfig/modules/dbbrowser/toolbar/toolbar.xml,dbbrowser/toolbar/toolbar.xml))
$(eval $(call gb_Package_add_file,dbaccess_uiconfig,xml/uiconfig/modules/dbquery/menubar/menubar.xml,dbquery/menubar/menubar.xml))
$(eval $(call gb_Package_add_file,dbaccess_uiconfig,xml/uiconfig/modules/dbquery/toolbar/designobjectbar.xml,dbquery/toolbar/designobjectbar.xml))
$(eval $(call gb_Package_add_file,dbaccess_uiconfig,xml/uiconfig/modules/dbquery/toolbar/sqlobjectbar.xml,dbquery/toolbar/sqlobjectbar.xml))
$(eval $(call gb_Package_add_file,dbaccess_uiconfig,xml/uiconfig/modules/dbquery/toolbar/toolbar.xml,dbquery/toolbar/toolbar.xml))
$(eval $(call gb_Package_add_file,dbaccess_uiconfig,xml/uiconfig/modules/dbrelation/menubar/menubar.xml,dbrelation/menubar/menubar.xml))
$(eval $(call gb_Package_add_file,dbaccess_uiconfig,xml/uiconfig/modules/dbrelation/toolbar/toolbar.xml,dbrelation/toolbar/toolbar.xml))
$(eval $(call gb_Package_add_file,dbaccess_uiconfig,xml/uiconfig/modules/dbtable/menubar/menubar.xml,dbtable/menubar/menubar.xml))
$(eval $(call gb_Package_add_file,dbaccess_uiconfig,xml/uiconfig/modules/dbtable/toolbar/toolbar.xml,dbtable/toolbar/toolbar.xml))
$(eval $(call gb_Package_add_file,dbaccess_uiconfig,xml/uiconfig/modules/dbtdata/menubar/menubar.xml,dbtdata/menubar/menubar.xml))
$(eval $(call gb_Package_add_file,dbaccess_uiconfig,xml/uiconfig/modules/dbtdata/toolbar/toolbar.xml,dbtdata/toolbar/toolbar.xml))

# vim: set noet sw=4 ts=4:
