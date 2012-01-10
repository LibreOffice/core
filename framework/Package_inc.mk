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



$(eval $(call gb_Package_Package,framework_inc,$(SRCDIR)/framework/inc))
$(eval $(call gb_Package_add_file,framework_inc,inc/framework/acceleratorinfo.hxx,framework/acceleratorinfo.hxx))
$(eval $(call gb_Package_add_file,framework_inc,inc/framework/actiontriggerhelper.hxx,framework/actiontriggerhelper.hxx))
$(eval $(call gb_Package_add_file,framework_inc,inc/framework/addonmenu.hxx,framework/addonmenu.hxx))
$(eval $(call gb_Package_add_file,framework_inc,inc/framework/addonsoptions.hxx,framework/addonsoptions.hxx))
$(eval $(call gb_Package_add_file,framework_inc,inc/framework/bmkmenu.hxx,framework/bmkmenu.hxx))
$(eval $(call gb_Package_add_file,framework_inc,inc/framework/configimporter.hxx,framework/configimporter.hxx))
$(eval $(call gb_Package_add_file,framework_inc,inc/framework/eventsconfiguration.hxx,framework/eventsconfiguration.hxx))
$(eval $(call gb_Package_add_file,framework_inc,inc/framework/framelistanalyzer.hxx,framework/framelistanalyzer.hxx))
$(eval $(call gb_Package_add_file,framework_inc,inc/framework/documentundoguard.hxx,framework/documentundoguard.hxx))
$(eval $(call gb_Package_add_file,framework_inc,inc/framework/undomanagerhelper.hxx,framework/undomanagerhelper.hxx))
$(eval $(call gb_Package_add_file,framework_inc,inc/framework/imutex.hxx,framework/imutex.hxx))
$(eval $(call gb_Package_add_file,framework_inc,inc/framework/iguard.hxx,framework/iguard.hxx))
$(eval $(call gb_Package_add_file,framework_inc,inc/framework/imageproducer.hxx,framework/imageproducer.hxx))
$(eval $(call gb_Package_add_file,framework_inc,inc/framework/interaction.hxx,framework/interaction.hxx))
$(eval $(call gb_Package_add_file,framework_inc,inc/framework/menuconfiguration.hxx,framework/menuconfiguration.hxx))
$(eval $(call gb_Package_add_file,framework_inc,inc/framework/menuextensionsupplier.hxx,framework/menuextensionsupplier.hxx))
$(eval $(call gb_Package_add_file,framework_inc,inc/framework/preventduplicateinteraction.hxx,framework/preventduplicateinteraction.hxx))
$(eval $(call gb_Package_add_file,framework_inc,inc/framework/sfxhelperfunctions.hxx,framework/sfxhelperfunctions.hxx))
$(eval $(call gb_Package_add_file,framework_inc,inc/framework/titlehelper.hxx,framework/titlehelper.hxx))
$(eval $(call gb_Package_add_file,framework_inc,inc/framework/toolboxconfiguration.hxx,framework/toolboxconfiguration.hxx))
$(eval $(call gb_Package_add_file,framework_inc,inc/framework/fwedllapi.h,framework/fwedllapi.h))
$(eval $(call gb_Package_add_file,framework_inc,inc/fwkdllapi.h,fwkdllapi.h))
$(eval $(call gb_Package_add_file,framework_inc,inc/fwidllapi.h,fwidllapi.h))
