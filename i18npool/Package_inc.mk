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



$(eval $(call gb_Package_Package,i18npool_inc,$(SRCDIR)/i18npool/inc))
$(eval $(call gb_Package_add_file,i18npool_inc,inc/i18npool/i18npooldllapi.h,i18npool/i18npooldllapi.h))
$(eval $(call gb_Package_add_file,i18npool_inc,inc/i18npool/lang.h,i18npool/lang.h))
$(eval $(call gb_Package_add_file,i18npool_inc,inc/i18npool/mslangid.hxx,i18npool/mslangid.hxx))
$(eval $(call gb_Package_add_file,i18npool_inc,inc/i18npool/paper.hxx,i18npool/paper.hxx))
