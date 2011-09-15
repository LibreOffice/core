#***************************************************************
# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
#  http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.
#***************************************************************

$(eval $(call gb_Library_Library,xsltdlg))

$(eval $(call gb_Library_set_componentfile,xsltdlg,filter/source/xsltdialog/xsltdlg))

$(eval $(call gb_Library_add_api,xsltdlg,\
	udkapi \
	offapi \
))

$(eval $(call gb_Library_set_include,xsltdlg,\
	-I$(SRCDIR)/filter/inc/pch \
	$$(INCLUDE) \
	-I$(SRCDIR)/filter/inc \
))

$(eval $(call gb_Library_add_linked_libs,xsltdlg,\
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
