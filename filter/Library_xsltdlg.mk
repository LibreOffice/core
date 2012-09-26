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

$(eval $(call gb_Library_Library,xsltdlg))

$(eval $(call gb_Library_set_componentfile,xsltdlg,filter/source/xsltdialog/xsltdlg))

$(eval $(call gb_Library_use_sdk_api,xsltdlg))

$(eval $(call gb_Library_set_include,xsltdlg,\
	$$(INCLUDE) \
	-I$(SRCDIR)/filter/inc \
))

$(eval $(call gb_Library_use_libraries,xsltdlg,\
	sfx \
	svt \
	tk \
	vcl \
	svl \
	utl \
	tl \
	comphelper \
	cppuhelper \
	cppu \
	sal \
	$(gb_UWINAPI) \
	$(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,xsltdlg,\
	filter/source/xsltdialog/typedetectionexport \
	filter/source/xsltdialog/typedetectionimport \
	filter/source/xsltdialog/xmlfileview \
	filter/source/xsltdialog/xmlfilterdialogcomponent \
	filter/source/xsltdialog/xmlfilterjar \
	filter/source/xsltdialog/xmlfiltersettingsdialog \
	filter/source/xsltdialog/xmlfiltertabdialog \
	filter/source/xsltdialog/xmlfiltertabpagebasic \
	filter/source/xsltdialog/xmlfiltertabpagexslt \
	filter/source/xsltdialog/xmlfiltertestdialog \
))

# vim: set noet sw=4 ts=4:
