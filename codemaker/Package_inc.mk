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



$(eval $(call gb_Package_Package,codemaker_inc,$(SRCDIR)/codemaker/inc))
$(eval $(call gb_Package_add_file,codemaker_inc,inc/codemaker/codemaker.hxx,codemaker/codemaker.hxx))
$(eval $(call gb_Package_add_file,codemaker_inc,inc/codemaker/commoncpp.hxx,codemaker/commoncpp.hxx))
$(eval $(call gb_Package_add_file,codemaker_inc,inc/codemaker/commonjava.hxx,codemaker/commonjava.hxx))
$(eval $(call gb_Package_add_file,codemaker_inc,inc/codemaker/dependencies.hxx,codemaker/dependencies.hxx))
$(eval $(call gb_Package_add_file,codemaker_inc,inc/codemaker/exceptiontree.hxx,codemaker/exceptiontree.hxx))
$(eval $(call gb_Package_add_file,codemaker_inc,inc/codemaker/generatedtypeset.hxx,codemaker/generatedtypeset.hxx))
$(eval $(call gb_Package_add_file,codemaker_inc,inc/codemaker/global.hxx,codemaker/global.hxx))
$(eval $(call gb_Package_add_file,codemaker_inc,inc/codemaker/options.hxx,codemaker/options.hxx))
$(eval $(call gb_Package_add_file,codemaker_inc,inc/codemaker/typemanager.hxx,codemaker/typemanager.hxx))
$(eval $(call gb_Package_add_file,codemaker_inc,inc/codemaker/unotype.hxx,codemaker/unotype.hxx))
