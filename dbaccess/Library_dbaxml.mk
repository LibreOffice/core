###############################################################
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
###############################################################



$(eval $(call gb_Library_Library,dbaxml))
$(eval $(call gb_Library_add_precompiled_header,dbaxml,$(SRCDIR)/dbaccess/inc/pch/precompiled_dbaxml))

$(eval $(call gb_Library_set_componentfile,dbaxml,dbaccess/source/filter/xml/dbaxml))

$(eval $(call gb_Library_add_api,dbaxml,\
	udkapi \
	offapi \
))

$(eval $(call gb_Library_set_include,dbaxml,\
	-I$(SRCDIR)/dbaccess/inc \
	-I$(SRCDIR)/dbaccess/source/filter/xml \
	-I$(SRCDIR)/dbaccess/source/inc \
	-I$(SRCDIR)/dbaccess/inc/pch \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_add_linked_libs,dbaxml,\
	tl \
	svl \
	cppuhelper \
	cppu \
	comphelper \
	utl \
	vcl \
	sal \
	sfx \
	dbtools \
	xo \
	sot \
	ootk \
	dba \
	stl \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,dbaxml,\
	dbaccess/source/filter/xml/xmlfilter                           \
	dbaccess/source/filter/xml/dbloader2                           \
	dbaccess/source/filter/xml/xmlDatabase                         \
	dbaccess/source/filter/xml/xmlDataSource                       \
	dbaccess/source/filter/xml/xmlTableFilterPattern       \
	dbaccess/source/filter/xml/xmlTableFilterList          \
	dbaccess/source/filter/xml/xmlDataSourceInfo           \
	dbaccess/source/filter/xml/xmlDataSourceSetting        \
	dbaccess/source/filter/xml/xmlDataSourceSettings       \
	dbaccess/source/filter/xml/xmlDocuments                        \
	dbaccess/source/filter/xml/xmlComponent                        \
	dbaccess/source/filter/xml/xmlHierarchyCollection      \
	dbaccess/source/filter/xml/xmlLogin                            \
	dbaccess/source/filter/xml/xmlExport                           \
	dbaccess/source/filter/xml/xmlQuery                            \
	dbaccess/source/filter/xml/xmlTable                            \
	dbaccess/source/filter/xml/xmlColumn                           \
	dbaccess/source/filter/xml/xmlStyleImport                      \
	dbaccess/source/filter/xml/xmlHelper                           \
	dbaccess/source/filter/xml/xmlAutoStyle                        \
	dbaccess/source/filter/xml/xmlConnectionData           \
	dbaccess/source/filter/xml/xmlDatabaseDescription      \
	dbaccess/source/filter/xml/xmlFileBasedDatabase        \
	dbaccess/source/filter/xml/xmlServerDatabase           \
	dbaccess/source/filter/xml/xmlConnectionResource       \
	dbaccess/source/filter/xml/xmlservices \
))

$(eval $(call gb_Library_add_noexception_objects,dbaxml, \
	dbaccess/source/shared/flt_reghelper \
	dbaccess/source/shared/xmlstrings \
))

# vim: set noet sw=4 ts=4:
