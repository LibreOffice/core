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



$(eval $(call gb_Library_Library,sdbt))
$(eval $(call gb_Library_add_package_headers,sdbt,dbaccess_inc))


$(eval $(call gb_Library_set_componentfile,sdbt,dbaccess/util/sdbt))

$(eval $(call gb_Library_add_api,sdbt,\
	udkapi \
	offapi \
))

$(eval $(call gb_Library_set_include,sdbt,\
	-I$(SRCDIR)/dbaccess/inc \
	-I$(SRCDIR)/dbaccess/source/sdbtools/inc \
	-I$(SRCDIR)/dbaccess/source/inc \
	-I$(SRCDIR)/dbaccess/inc/pch \
	$$(INCLUDE) \
))

$(eval $(call gb_Library_add_linked_libs,sdbt,\
	tl \
	cppuhelper \
	cppu \
	comphelper \
	utl \
	sal \
	stl \
	dbtools \
    $(gb_STDLIBS) \
))

$(eval $(call gb_Library_add_exception_objects,sdbt,\
	dbaccess/source/sdbtools/connection/connectiontools \
	dbaccess/source/sdbtools/connection/tablename \
	dbaccess/source/sdbtools/connection/objectnames \
	dbaccess/source/sdbtools/connection/datasourcemetadata \
	dbaccess/source/sdbtools/misc/sdbt_services \
	dbaccess/source/sdbtools/misc/module_sdbt \
))

$(eval $(call gb_Library_add_noexception_objects,sdbt, \
	dbaccess/source/shared/sdbtstrings \
))

# vim: set noet sw=4 ts=4:
