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

$(eval $(call gb_AllLangResTarget_AllLangResTarget,xsltdlg))

$(eval $(call gb_AllLangResTarget_add_srs,xsltdlg,\
	filter/xsltdlg \
))

$(eval $(call gb_SrsTarget_SrsTarget,filter/xsltdlg))

$(eval $(call gb_SrsTarget_set_include,filter/xsltdlg,\
	$$(INCLUDE) \
	-I$(SRCDIR)/filter/inc \
))

$(eval $(call gb_SrsTarget_add_files,filter/xsltdlg,\
	filter/source/xsltdialog/xmlfiltersettingsdialog.src \
	filter/source/xsltdialog/xmlfiltertabdialog.src \
	filter/source/xsltdialog/xmlfiltertabpagebasic.src \
	filter/source/xsltdialog/xmlfiltertabpagexslt.src \
	filter/source/xsltdialog/xmlfiltertestdialog.src \
	filter/source/xsltdialog/xmlfileview.src \
	filter/source/xsltdialog/xmlfilterdialogstrings.src \
))

# vim: set noet sw=4 ts=4:
