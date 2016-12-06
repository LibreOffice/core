#**************************************************************
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
#**************************************************************



$(eval $(call gb_Library_Library,uui))

$(eval $(call gb_Library_set_componentfile,uui,uui/util/uui))

$(eval $(call gb_Library_set_include,uui,\
        $$(INCLUDE) \
))

$(eval $(call gb_Library_add_api,uui,\
	offapi \
	udkapi \
))

$(eval $(call gb_Library_add_linked_libs,uui,\
	comphelper \
	cppu \
	cppuhelper \
	ootk \
	sal \
	svl \
	svt \
	tl \
	utl \
	vcl \
	$(gb_STDLIBS) \
))


$(eval $(call gb_Library_add_exception_objects,uui,\
	uui/source/cookiedg \
	uui/source/iahndl \
	uui/source/iahndl-authentication \
	uui/source/iahndl-ssl \
	uui/source/iahndl-cookies \
	uui/source/iahndl-filter \
	uui/source/iahndl-locking \
	uui/source/iahndl-ioexceptions \
	uui/source/iahndl-errorhandler \
	uui/source/logindlg \
	uui/source/services \
	uui/source/masterpassworddlg \
	uui/source/masterpasscrtdlg \
	uui/source/openlocked \
	uui/source/passworddlg \
	uui/source/fltdlg \
	uui/source/interactionhandler \
	uui/source/requeststringresolver \
	uui/source/unknownauthdlg \
	uui/source/sslwarndlg \
	uui/source/secmacrowarnings \
	uui/source/filechanged \
	uui/source/alreadyopen \
	uui/source/lockfailed \
	uui/source/trylater \
	uui/source/newerverwarn \
	uui/source/nameclashdlg \
	uui/source/passwordcontainer \
))

# vim: set noet sw=4 ts=4:
