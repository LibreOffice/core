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



$(eval $(call gb_Library_Library,dbmm))
$(eval $(call gb_Library_add_package_headers,dbmm,dbaccess_inc))
$(eval $(call gb_Library_add_precompiled_header,dbmm,$(SRCDIR)/dbaccess/inc/pch/precompiled_dbmm))

$(eval $(call gb_Library_set_componentfile,dbmm,dbaccess/source/ext/macromigration/dbmm))

$(eval $(call gb_Library_add_api,dbmm,\
	udkapi \
	offapi \
))

$(eval $(call gb_Library_set_include,dbmm,\
	-I$(SRCDIR)/dbaccess/inc \
	-I$(SRCDIR)/dbaccess/source/ext/macromigration \
	-I$(SRCDIR)/dbaccess/inc/pch \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_add_linked_libs,dbmm,\
	cppu          \
    cppuhelper    \
    comphelper    \
    utl      \
    tl         \
	sal           \
	svt        \
	svl           \
	vcl           \
	svxcore           \
	svx           \
	ucbhelper     \
	stl \
	xcr \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,dbmm,\
	dbaccess/source/ext/macromigration/macromigrationwizard \
	dbaccess/source/ext/macromigration/macromigrationdialog \
	dbaccess/source/ext/macromigration/macromigrationpages \
	dbaccess/source/ext/macromigration/dbmm_module \
	dbaccess/source/ext/macromigration/dbmm_services \
	dbaccess/source/ext/macromigration/migrationengine \
	dbaccess/source/ext/macromigration/docinteraction \
	dbaccess/source/ext/macromigration/progresscapture \
	dbaccess/source/ext/macromigration/progressmixer \
	dbaccess/source/ext/macromigration/migrationlog \
	dbaccess/source/ext/macromigration/dbmm_types \
))

# vim: set noet sw=4 ts=4:
