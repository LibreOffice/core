# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
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
# Copyright (C) 2012 Red Hat, Inc., Caolán McNamara <caolanm@redhat.com>
#  (initial developer)
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

$(eval $(call gb_Package_Package,test_inc,$(SRCDIR)/test/inc))
$(eval $(call gb_Package_add_file,test_inc,inc/test/xmldiff.hxx,test/xmldiff.hxx))
$(eval $(call gb_Package_add_file,test_inc,inc/test/bootstrapfixture.hxx,test/bootstrapfixture.hxx))
$(eval $(call gb_Package_add_file,test_inc,inc/test/testdllapi.hxx,test/testdllapi.hxx))
$(eval $(call gb_Package_add_file,test_inc,inc/test/unoapi_test.hxx,test/unoapi_test.hxx))
$(eval $(call gb_Package_add_file,test_inc,inc/test/beans/xpropertyset.hxx,test/beans/xpropertyset.hxx))
$(eval $(call gb_Package_add_file,test_inc,inc/test/sheet/xdatapilotdescriptor.hxx,test/sheet/xdatapilotdescriptor.hxx))
$(eval $(call gb_Package_add_file,test_inc,inc/test/sheet/xcellrangesquery.hxx,test/sheet/xcellrangesquery.hxx))
$(eval $(call gb_Package_add_file,test_inc,inc/test/sheet/xgoalseek.hxx,test/sheet/xgoalseek.hxx))
$(eval $(call gb_Package_add_file,test_inc,inc/test/sheet/xnamedranges.hxx,test/sheet/xnamedranges.hxx))
$(eval $(call gb_Package_add_file,test_inc,inc/test/sheet/xnamedrange.hxx,test/sheet/xnamedrange.hxx))
$(eval $(call gb_Package_add_file,test_inc,inc/test/sheet/xdatabaserange.hxx,test/sheet/xdatabaserange.hxx))
$(eval $(call gb_Package_add_file,test_inc,inc/test/container/xelementaccess.hxx,test/container/xelementaccess.hxx))
$(eval $(call gb_Package_add_file,test_inc,inc/test/container/xnamed.hxx,test/container/xnamed.hxx))
$(eval $(call gb_Package_add_file,test_inc,inc/test/container/xindexaccess.hxx,test/container/xindexaccess.hxx))
$(eval $(call gb_Package_add_file,test_inc,inc/test/container/xnamecontainer.hxx,test/container/xnamecontainer.hxx))
$(eval $(call gb_Package_add_file,test_inc,inc/test/container/xnamereplace.hxx,test/container/xnamereplace.hxx))
$(eval $(call gb_Package_add_file,test_inc,inc/test/sheet/cellproperties.hxx,test/sheet/cellproperties.hxx))
$(eval $(call gb_Package_add_file,test_inc,inc/test/sheet/xcellrangedata.hxx,test/sheet/xcellrangedata.hxx))
$(eval $(call gb_Package_add_file,test_inc,inc/test/sheet/xcellrangereferrer.hxx,test/sheet/xcellrangereferrer.hxx))
$(eval $(call gb_Package_add_file,test_inc,inc/test/sheet/xdatapilottable.hxx,test/sheet/xdatapilottable.hxx))
$(eval $(call gb_Package_add_file,test_inc,inc/test/sheet/xdatapilottable2.hxx,test/sheet/xdatapilottable2.hxx))
$(eval $(call gb_Package_add_file,test_inc,inc/test/sheet/xdatapilotfieldgrouping.hxx,test/sheet/xdatapilotfieldgrouping.hxx))
$(eval $(call gb_Package_add_file,test_inc,inc/test/sheet/datapilotfield.hxx,test/sheet/datapilotfield.hxx))
$(eval $(call gb_Package_add_file,test_inc,inc/test/sheet/xprintareas.hxx,test/sheet/xprintareas.hxx))
$(eval $(call gb_Package_add_file,test_inc,inc/test/sheet/xspreadsheetdocument.hxx,test/sheet/xspreadsheetdocument.hxx))
$(eval $(call gb_Package_add_file,test_inc,inc/test/sheet/xspreadsheets2.hxx,test/sheet/xspreadsheets2.hxx))
$(eval $(call gb_Package_add_file,test_inc,inc/test/text/xtext.hxx,test/text/xtext.hxx))
$(eval $(call gb_Package_add_file,test_inc,inc/test/text/xtextfield.hxx,test/text/xtextfield.hxx))
$(eval $(call gb_Package_add_file,test_inc,inc/test/text/xtextcontent.hxx,test/text/xtextcontent.hxx))
$(eval $(call gb_Package_add_file,test_inc,inc/test/util/xreplaceable.hxx,test/util/xreplaceable.hxx))
$(eval $(call gb_Package_add_file,test_inc,inc/test/util/xsearchable.hxx,test/util/xsearchable.hxx))

# vim: set noet sw=4 ts=4:
