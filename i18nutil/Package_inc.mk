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



$(eval $(call gb_Package_Package,i18nutil_inc,$(SRCDIR)/i18nutil/inc/i18nutil))

$(eval $(call gb_Package_add_file,i18nutil_inc,inc/i18nutil/i18nutildllapi.h,i18nutildllapi.h))
$(eval $(call gb_Package_add_file,i18nutil_inc,inc/i18nutil/casefolding.hxx,casefolding.hxx))
$(eval $(call gb_Package_add_file,i18nutil_inc,inc/i18nutil/oneToOneMapping.hxx,oneToOneMapping.hxx))
$(eval $(call gb_Package_add_file,i18nutil_inc,inc/i18nutil/unicode.hxx,unicode.hxx))
$(eval $(call gb_Package_add_file,i18nutil_inc,inc/i18nutil/widthfolding.hxx,widthfolding.hxx))
$(eval $(call gb_Package_add_file,i18nutil_inc,inc/i18nutil/x_rtl_ustring.h,x_rtl_ustring.h))
