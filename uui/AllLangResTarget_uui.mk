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



$(eval $(call gb_AllLangResTarget_AllLangResTarget,uui))

$(eval $(call gb_AllLangResTarget_add_srs,uui,\
	uui/res \
))

$(eval $(call gb_SrsTarget_SrsTarget,uui/res))

$(eval $(call gb_SrsTarget_set_include,uui/res,\
	$$(INCLUDE) \
	-I$(OUTDIR)/inc \
	-I$(SRCDIR)/uui/source \
))

$(eval $(call gb_SrsTarget_add_files,uui/res,\
	uui/source/cookiedg.src \
	uui/source/ids.src \
	uui/source/logindlg.src \
	uui/source/masterpassworddlg.src \
	uui/source/masterpasscrtdlg.src \
	uui/source/openlocked.src \
	uui/source/passworddlg.src \
	uui/source/passworderrs.src \
	uui/source/fltdlg.src \
	uui/source/unknownauthdlg.src \
	uui/source/sslwarndlg.src \
	uui/source/secmacrowarnings.src \
	uui/source/filechanged.src \
	uui/source/alreadyopen.src \
	uui/source/lockfailed.src \
	uui/source/trylater.src \
	uui/source/nameclashdlg.src \
	uui/source/newerverwarn.src \
))
