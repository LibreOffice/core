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



$(eval $(call gb_AllLangResTarget_AllLangResTarget,avmedia))

$(eval $(call gb_AllLangResTarget_add_srs,avmedia,avmedia/res))

$(eval $(call gb_SrsTarget_SrsTarget,avmedia/res))

$(eval $(call gb_SrsTarget_set_include,avmedia/res,\
	$$(INCLUDE) \
	-I$(SRCDIR)/avmedia/inc \
))

ifeq ($(OS),MACOSX)
$(eval $(call gb_Library_add_cxxflags,avmedia,\
    -x objective-c++ \
))
endif

# add src files here (complete path relative to repository root)
$(eval $(call gb_SrsTarget_add_files,avmedia/res,\
	avmedia/source/framework/mediacontrol.src \
	avmedia/source/viewer/mediawindow.src \
))

# vim: set noet sw=4 ts=4:
