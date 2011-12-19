# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
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
# Major Contributor(s):
# Copyright (C) 2011 Peter Foley <pefoley2@verizon.net> (initial developer)
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_Package_Package,connectivity_xml,$(SRCDIR)/connectivity/source))

$(eval $(call gb_Package_add_file,connectivity_xml,xml/evoab.xml,drivers/evoab2/evoab.xml))
$(eval $(call gb_Package_add_file,connectivity_xml,xml/odbc.xml,drivers/odbc/odbc.xml))
$(eval $(call gb_Package_add_file,connectivity_xml,xml/dbase.xml,drivers/dbase/dbase.xml))
$(eval $(call gb_Package_add_file,connectivity_xml,xml/kab.xml,drivers/kab/kab.xml))
$(eval $(call gb_Package_add_file,connectivity_xml,xml/mozab.xml,drivers/mozab/mozab.xml))
$(eval $(call gb_Package_add_file,connectivity_xml,xml/mysql.xml,drivers/mysql/mysql.xml))
$(eval $(call gb_Package_add_file,connectivity_xml,xml/flat.xml,drivers/flat/flat.xml))
$(eval $(call gb_Package_add_file,connectivity_xml,xml/file.xml,drivers/file/file.xml))
$(eval $(call gb_Package_add_file,connectivity_xml,xml/CalcDriver.xml,drivers/calc/CalcDriver.xml))
$(eval $(call gb_Package_add_file,connectivity_xml,xml/jdbc.xml,drivers/jdbc/jdbc.xml))
$(eval $(call gb_Package_add_file,connectivity_xml,xml/macab.xml,drivers/macab/macab.xml))
$(eval $(call gb_Package_add_file,connectivity_xml,xml/ado.xml,drivers/ado/ado.xml))
$(eval $(call gb_Package_add_file,connectivity_xml,xml/dbpool.xml,cpool/dbpool.xml))

# vim: set noet sw=4 ts=4:
