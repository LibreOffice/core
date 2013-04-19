# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

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
$(eval $(call gb_Package_add_file,test_inc,inc/test/sheet/xsheetannotation.hxx,test/sheet/xsheetannotation.hxx))
$(eval $(call gb_Package_add_file,test_inc,inc/test/sheet/xsheetannotations.hxx,test/sheet/xsheetannotations.hxx))
$(eval $(call gb_Package_add_file,test_inc,inc/test/sheet/xsheetoutline.hxx,test/sheet/xsheetoutline.hxx))
$(eval $(call gb_Package_add_file,test_inc,inc/test/text/xtext.hxx,test/text/xtext.hxx))
$(eval $(call gb_Package_add_file,test_inc,inc/test/text/xtextfield.hxx,test/text/xtextfield.hxx))
$(eval $(call gb_Package_add_file,test_inc,inc/test/text/xtextcontent.hxx,test/text/xtextcontent.hxx))
$(eval $(call gb_Package_add_file,test_inc,inc/test/util/xreplaceable.hxx,test/util/xreplaceable.hxx))
$(eval $(call gb_Package_add_file,test_inc,inc/test/util/xsearchable.hxx,test/util/xsearchable.hxx))

# vim: set noet sw=4 ts=4:
