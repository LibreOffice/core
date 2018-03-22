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



$(eval $(call gb_Package_Package,registry_inc,$(SRCDIR)/registry/inc))
$(eval $(call gb_Package_add_file,registry_inc,inc/registry/reader.h,registry/reader.h))
$(eval $(call gb_Package_add_file,registry_inc,inc/registry/reader.hxx,registry/reader.hxx))
$(eval $(call gb_Package_add_file,registry_inc,inc/registry/reflread.hxx,registry/reflread.hxx))
$(eval $(call gb_Package_add_file,registry_inc,inc/registry/refltype.hxx,registry/refltype.hxx))
$(eval $(call gb_Package_add_file,registry_inc,inc/registry/reflwrit.hxx,registry/reflwrit.hxx))
$(eval $(call gb_Package_add_file,registry_inc,inc/registry/registry.h,registry/registry.h))
$(eval $(call gb_Package_add_file,registry_inc,inc/registry/registry.hxx,registry/registry.hxx))
$(eval $(call gb_Package_add_file,registry_inc,inc/registry/registrydllapi.h,registry/registrydllapi.h))
$(eval $(call gb_Package_add_file,registry_inc,inc/registry/regtype.h,registry/regtype.h))
$(eval $(call gb_Package_add_file,registry_inc,inc/registry/types.h,registry/types.h))
$(eval $(call gb_Package_add_file,registry_inc,inc/registry/version.h,registry/version.h))
$(eval $(call gb_Package_add_file,registry_inc,inc/registry/writer.h,registry/writer.h))
$(eval $(call gb_Package_add_file,registry_inc,inc/registry/writer.hxx,registry/writer.hxx))
